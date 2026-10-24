#include "engine/server_engine.h"

#include "common/global_settings.hpp"
#include "common/log.hpp"
#include "protocol/protocol.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>
#include <type_traits>

namespace {

std::atomic<botf::ServerEngine*> gActiveEngine{nullptr};

void onSignal(int) {
	if (botf::ServerEngine* engine = gActiveEngine.load()) {
		engine->requestStop();
	}
}

botf::common::WorldSnapshot buildWorldSnapshot(const botf::GameSession& session,
											   std::uint32_t tick) {
	const botf::GameLogic& game = session.logic();
	botf::common::WorldSnapshot snapshot;
	snapshot.tick = tick;
	snapshot.mapId = session.mapId();
	snapshot.figures.reserve(game.figures().size());
	for (const auto& obj : game.figures()) {
		snapshot.figures.push_back(botf::common::FigurePose{
			obj.getObjectId(),
			obj.getPosition(),
			obj.getAngle(),
			obj.getHp(),
			obj.getLifeStatus(),
		});
	}
	snapshot.bullets.reserve(game.bullets().size());
	for (const auto& bullet : game.bullets()) {
		snapshot.bullets.push_back(botf::common::BulletPose{
			bullet.getObjectId(),
			bullet.getOwnerId(),
			bullet.getPosition(),
			bullet.getVelocity(),
		});
	}
	return snapshot;
}

}  // namespace

botf::ServerEngine::ServerEngine() : m_server(m_inbound, m_outbound) {}

botf::ServerEngine::~ServerEngine() {
	requestStop();
	shutdown();
	if (gActiveEngine.load() == this) {
		gActiveEngine.store(nullptr);
	}
}

void botf::ServerEngine::requestStop() {
	m_running = false;
}

bool botf::ServerEngine::startListen(std::uint16_t port) {
	return m_server.listen(port);
}

void botf::ServerEngine::shutdown() {
	m_server.stop();
}

void botf::ServerEngine::pollNetwork() {
	m_server.poll();
	for (const common::ClientID clientId : m_server.takeDisconnected()) {
		m_sessions.removeClient(clientId);
		m_inputStates.erase(clientId);
		botf::log::info("[server] removed entity for disconnected client {}", clientId);
	}
}

std::uint16_t botf::ServerEngine::port() const {
	return m_server.port();
}

std::size_t botf::ServerEngine::clientCount() const {
	return m_server.clientCount();
}

botf::GameSessionsPool& botf::ServerEngine::sessions() {
	return m_sessions;
}

const botf::GameSessionsPool& botf::ServerEngine::sessions() const {
	return m_sessions;
}

const botf::GameLogic& botf::ServerEngine::game() const {
	return m_sessions.sessions().front()->logic();
}

std::uint32_t botf::ServerEngine::simTick() const {
	return m_simTick;
}

std::optional<botf::common::ClientInput> botf::ServerEngine::lastClientInput(
	common::ClientID clientId) const {
	const auto it = m_inputStates.find(clientId);
	if (it == m_inputStates.end()) {
		return std::nullopt;
	}
	return it->second.command;
}

botf::common::ObjectID botf::ServerEngine::bindOrSpawnForClient(GameSession& session,
															   common::ClientID clientId) {
	GameLogic& game = session.logic();
	if (const auto existing = game.entityForClient(clientId)) {
		return *existing;
	}

	const common::ObjectID entity = game.spawnPlayer(
		common::Point{common::GlobalSettings::defaultSpawnX, common::GlobalSettings::defaultSpawnY},
		common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(clientId, entity);
	botf::log::info("[server] session {} bound to entity {} (mapId={})", clientId, entity,
					session.mapId());
	return entity;
}

void botf::ServerEngine::handleClientConnect(common::ClientID clientId,
											 const common::ClientConnectMessage& connect,
											 std::uint64_t sequence) {
	// Card 46 scaffold: TCP login still enters the sole GameSession.
	// Later (45/47): ClientConnect = server login only; JoinSession notification joins match.
	GameSession& session = m_sessions.ensureDefaultFor(clientId);
	const common::ObjectID entityId = bindOrSpawnForClient(session, clientId);
	const common::ClientConnectMessage ack{connect.playerName, entityId};
	m_outbound.push(net::OutboundNotification{
		clientId,
		protocol::encodeClientConnect(ack, sequence),
	});
}

void botf::ServerEngine::handleClientInput(common::ClientID clientId,
										   const common::ClientInput& input,
										   std::uint64_t sequence) {
	m_inputStates[clientId] = ClientInputState{input, sequence};

	GameSession* session = m_sessions.findByClient(clientId);
	if (!session) {
		// Unbound ClientInput (no ClientConnect) — no-op.
		return;
	}

	GameLogic& game = session->logic();
	game.setClientInput(clientId, input.dx, input.dy);
	game.setClientAim(clientId, input.aim);
	if (input.fire) {
		game.tryFire(clientId);
	}
}

void botf::ServerEngine::applyInbound() {
	while (auto note = m_inbound.tryPop()) {
		auto decoded = protocol::decode(note->payload);
		if (!decoded) {
			botf::log::error("[server] bad envelope from client {}", note->clientId);
			continue;
		}

		const common::ClientID clientId = note->clientId;
		protocol::dispatch(*decoded, [this, clientId](const auto& body, std::uint64_t sequence) {
			using Body = std::decay_t<decltype(body)>;
			if constexpr (std::is_same_v<Body, common::ClientConnectMessage>) {
				handleClientConnect(clientId, body, sequence);
			} else if constexpr (std::is_same_v<Body, common::ClientInput>) {
				handleClientInput(clientId, body, sequence);
			}
			// WorldSnapshot from a client is ignored.
		});
	}
}

void botf::ServerEngine::enqueueWorldState(const GameSession& session,
										   std::uint32_t tick,
										   std::uint64_t sequence) {
	const auto payload = protocol::encodeWorldSnapshot(buildWorldSnapshot(session, tick), sequence);
	// Card 46: one shared session — broadcast to all. Per-session fan-out ready for 47.
	if (session.clients().empty()) {
		return;
	}
	for (const common::ClientID clientId : session.clients()) {
		m_outbound.push(net::OutboundNotification{clientId, payload});
	}
}

void botf::ServerEngine::advanceSim(float dt) {
	++m_simTick;
	for (auto& session : m_sessions.sessions()) {
		session->logic().tick(dt);
		enqueueWorldState(*session, m_simTick, m_simTick);
	}
}

int botf::ServerEngine::runLoop() {
	botf::log::info("[server] running on port {} at {} Hz (Ctrl+C to stop)", port(),
					common::GlobalSettings::kTickHz);

	m_running = true;
	auto lastOverrunLog = std::chrono::steady_clock::time_point{};

	while (m_running) {
		const auto tickStart = std::chrono::steady_clock::now();

		pollNetwork();
		applyInbound();
		advanceSim(common::GlobalSettings::kDt);
		pollNetwork();

		// tick overrun
		const auto elapsed = std::chrono::steady_clock::now() - tickStart;
		if (elapsed > common::GlobalSettings::kTickPeriod) {
			constexpr auto kOverrunLogCooldown = std::chrono::seconds(1);
			if (tickStart - lastOverrunLog >= kOverrunLogCooldown) {
				const auto ms =
					std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
				botf::log::warn("[server] tick overrun: {} ms (budget {} ms)", ms,
								common::GlobalSettings::kTickPeriod.count());
				lastOverrunLog = tickStart;
			}
		} else {
			std::this_thread::sleep_for(common::GlobalSettings::kTickPeriod - elapsed);
		}
	}

	botf::log::info("[server] shutting down");
	return 0;
}

int botf::ServerEngine::run(std::uint16_t port) {
	botf::log::init();

	gActiveEngine.store(this);
	std::signal(SIGINT, onSignal);
	std::signal(SIGTERM, onSignal);

	if (!startListen(port)) {
		gActiveEngine.store(nullptr);
		return 1;
	}

	const int code = runLoop();
	shutdown();
	gActiveEngine.store(nullptr);
	return code;
}
