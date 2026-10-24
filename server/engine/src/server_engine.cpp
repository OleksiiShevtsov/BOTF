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
	const botf::GameLogic& game = session.getGameLogic();
	botf::common::WorldSnapshot snapshot;
	snapshot.tick = tick;
	snapshot.figures.reserve(game.figures().size());
	for (const auto& figure : game.figures()) {
		snapshot.figures.push_back(botf::common::FigurePose{
			figure.getId(),
			figure.getPosition(),
			figure.getAngle(),
			figure.getHp(),
			figure.getLifeStatus(),
		});
	}
	snapshot.bullets.reserve(game.bullets().size());
	for (const auto& bullet : game.bullets()) {
		snapshot.bullets.push_back(botf::common::BulletPose{
			bullet.getId(),
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
	for (const common::ID clientId : m_server.takeDisconnected()) {
		m_gameSessionsPool.removeClient(clientId);
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

botf::GameSessionsPool& botf::ServerEngine::getGameSessionsPool() {
	return m_gameSessionsPool;
}

const botf::GameSessionsPool& botf::ServerEngine::getGameSessionsPool() const {
	return m_gameSessionsPool;
}

std::uint32_t botf::ServerEngine::simTick() const {
	return m_simTick;
}

std::optional<botf::common::ClientInput> botf::ServerEngine::lastClientInput(
	common::ID clientId) const {
	const auto it = m_inputStates.find(clientId);
	if (it == m_inputStates.end()) {
		return std::nullopt;
	}
	return it->second.command;
}

botf::common::ID botf::ServerEngine::bindOrSpawnForClient(common::ID clientId) {
	GameSession* session = m_gameSessionsPool.findGameSessionByClientId(clientId);
	if (!session) {
		botf::log::error("[server] bindOrSpawn: no session for client {}", clientId);
		return 0;
	}

	GameLogic& game = session->getGameLogic();
	if (const auto existing = game.entityForClient(clientId)) {
		return *existing;
	}

	const common::ID entity = game.spawnPlayer(
		common::Point{common::GlobalSettings::defaultSpawnX, common::GlobalSettings::defaultSpawnY},
		common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(clientId, entity);
	botf::log::info("[server] client {} bound to entity {} (sessionId={})", clientId, entity,
					session->getId());
	return entity;
}

void botf::ServerEngine::handleClientConnect(common::ID clientId,
											 const common::ClientConnectMessage& connect,
											 std::uint64_t sequence) {
	// Card 46 scaffold: TCP login still enters the sole GameSession.
	// Later (45/47): ClientConnect = server login only; JoinSession notification joins match.
	m_gameSessionsPool.ensureDefaultFor(clientId);
	const common::ID entityId = bindOrSpawnForClient(clientId);
	const common::ClientConnectMessage ack{connect.playerName, entityId};
	m_outbound.push(net::OutboundNotification{
		clientId,
		protocol::encodeClientConnect(ack, sequence),
	});
}

void botf::ServerEngine::handleClientInput(common::ID clientId,
										   const common::ClientInput& input,
										   std::uint64_t sequence) {
	m_inputStates[clientId] = ClientInputState{input, sequence};

	GameSession* session = m_gameSessionsPool.findGameSessionByClientId(clientId);
	if (!session) {
		// Unbound ClientInput (no ClientConnect) — no-op.
		return;
	}

	GameLogic& game = session->getGameLogic();
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

		const common::ID clientId = note->clientId;
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
	// Fan-out to clients mapped to this session (membership in GameSessionsPool).
	for (const common::ID clientId : m_gameSessionsPool.getSessionClients(session.getId())) {
		m_outbound.push(net::OutboundNotification{clientId, payload});
	}
}

void botf::ServerEngine::advanceSim(float dt) {
	++m_simTick;
	for (auto& [sessionId, session] : m_gameSessionsPool.sessions()) {
		(void)sessionId;
		session->getGameLogic().tick(dt);
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
