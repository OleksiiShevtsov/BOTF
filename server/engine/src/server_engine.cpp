#include "engine/server_engine.h"

#include "common/global_settings.hpp"
#include "common/log.hpp"
#include "protocol/protocol.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>
#include <variant>

namespace {

constexpr int kTickHz = 20;
constexpr float kDt = 1.0f / static_cast<float>(kTickHz);
constexpr auto kTickPeriod = std::chrono::milliseconds(1000 / kTickHz);

std::atomic<botf::ServerEngine*> gActiveEngine{nullptr};

void onSignal(int) {
	if (botf::ServerEngine* engine = gActiveEngine.load()) {
		engine->requestStop();
	}
}

botf::common::WorldSnapshot buildWorldSnapshot(const botf::GameLogic& game, std::uint32_t tick) {
	botf::common::WorldSnapshot snapshot;
	snapshot.tick = tick;
	snapshot.players.reserve(game.objects().size());
	for (const auto& obj : game.objects()) {
		snapshot.players.push_back(botf::common::PlayerPose{obj.id(), obj.position(), obj.angle()});
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
}

std::uint16_t botf::ServerEngine::port() const {
	return m_server.port();
}

std::size_t botf::ServerEngine::clientCount() const {
	return m_server.clientCount();
}

const botf::GameLogic& botf::ServerEngine::game() const {
	return m_game;
}

std::uint32_t botf::ServerEngine::simTick() const {
	return m_simTick;
}

std::optional<botf::common::MoveCommand> botf::ServerEngine::lastMove(
	common::ClientID clientId) const {
	const auto it = m_inputStates.find(clientId);
	if (it == m_inputStates.end()) {
		return std::nullopt;
	}
	return it->second.command;
}

botf::common::ObjectID botf::ServerEngine::bindOrSpawnForClient(common::ClientID clientId) {
	if (const auto existing = m_game.entityForClient(clientId)) {
		return *existing;
	}

	const common::ObjectID entity = m_game.spawnPlayer(
		common::Point{common::GlobalSettings::defaultSpawnX, common::GlobalSettings::defaultSpawnY},
		common::GlobalSettings::defaultPlayerSpeedLimit);
	m_game.bindClient(clientId, entity);
	botf::log::info("[server] session {} bound to entity {}", clientId, entity);
	return entity;
}

void botf::ServerEngine::handleHello(common::ClientID clientId,
									 const common::HelloMessage& hello,
									 std::uint64_t sequence) {
	const common::ObjectID entityId = bindOrSpawnForClient(clientId);
	const common::HelloMessage ack{hello.playerName, entityId};
	m_outbound.push(net::OutboundNotification{
		clientId,
		protocol::encodeHello(ack, sequence),
	});
}

void botf::ServerEngine::handleMove(common::ClientID clientId,
									const common::MoveCommand& command,
									std::uint64_t sequence) {
	m_inputStates[clientId] = ClientInputState{command, sequence};

	// Move applies only to the entity bound via Hello. Unbound → no-op.
	m_game.setClientInput(clientId, command.dx, command.dy);
}

void botf::ServerEngine::applyInbound() {
	while (auto note = m_inbound.tryPop()) {
		auto decoded = protocol::decode(note->payload);
		if (!decoded) {
			botf::log::error("[server] bad envelope from client {}", note->clientId);
			continue;
		}

		if (const auto* hello = std::get_if<protocol::DecodedHello>(&*decoded)) {
			handleHello(note->clientId, hello->hello, hello->sequence);
			continue;
		}

		if (const auto* move = std::get_if<protocol::DecodedMove>(&*decoded)) {
			handleMove(note->clientId, move->command, move->sequence);
			continue;
		}
	}
}

void botf::ServerEngine::enqueueWorldState(std::uint32_t tick, std::uint64_t sequence) {
	m_outbound.push(net::OutboundNotification{
		std::nullopt,
		protocol::encodeWorldSnapshot(buildWorldSnapshot(m_game, tick), sequence),
	});
}

void botf::ServerEngine::advanceSim(float dt) {
	m_game.tick(dt);
	++m_simTick;
	enqueueWorldState(m_simTick, m_simTick);
}

int botf::ServerEngine::runLoop() {
	botf::log::info("[server] running on port {} at {} Hz (Ctrl+C to stop)", port(), kTickHz);

	m_running = true;
	auto lastOverrunLog = std::chrono::steady_clock::time_point{};

	while (m_running) {
		const auto tickStart = std::chrono::steady_clock::now();

		pollNetwork();
		applyInbound();
		advanceSim(kDt);
		pollNetwork();
		
		// tick overrun
		const auto elapsed = std::chrono::steady_clock::now() - tickStart;
		if (elapsed > kTickPeriod) {
			constexpr auto kOverrunLogCooldown = std::chrono::seconds(1);
			if (tickStart - lastOverrunLog >= kOverrunLogCooldown) {
				const auto ms =
					std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
				botf::log::warn("[server] tick overrun: {} ms (budget {} ms)", ms,
								kTickPeriod.count());
				lastOverrunLog = tickStart;
			}
		} else {
			std::this_thread::sleep_for(kTickPeriod - elapsed);
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
