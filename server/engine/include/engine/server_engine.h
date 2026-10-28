#pragma once

#include "common/dto.hpp"
#include "engine/game_map_pool.h"
#include "engine/game_sessions_pool.h"
#include "net/tcp_server.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>

namespace botf {

// Server runtime: TCP + content catalogs + GameSessionsPool tick + WorldState broadcast.
// main should only parse args and call run().
class ServerEngine {
public:
	ServerEngine();
	~ServerEngine();

	ServerEngine(const ServerEngine&) = delete;
	ServerEngine& operator=(const ServerEngine&) = delete;

	// Listen and run the 20 Hz loop until signal / stop.
	int run(std::uint16_t port = 54545);

	void requestStop();

	// Orchestration steps shared by run() and integration tests.
	bool startListen(std::uint16_t port);
	void shutdown();
	void pollNetwork();
	// ClientConnect (spawn/bind + ack), ClientInput (own entity only).
	void applyInbound();
	// tick all sessions + enqueue WorldState for the new sim tick.
	void advanceSim(float dt);

	[[nodiscard]] std::uint16_t port() const;
	[[nodiscard]] std::size_t clientCount() const;
	[[nodiscard]] GameMapPool& getGameMapPool();
	[[nodiscard]] const GameMapPool& getGameMapPool() const;
	[[nodiscard]] GameSessionsPool& getGameSessionsPool();
	[[nodiscard]] const GameSessionsPool& getGameSessionsPool() const;
	[[nodiscard]] std::uint32_t simTick() const;
	[[nodiscard]] std::optional<common::ClientInput> lastClientInput(
		const common::ID clientId) const;

private:
	struct ClientInputState {
		common::ClientInput command{};
		std::uint64_t sequence = 0;
	};

	// Content catalogs at engine start (maps now; more domains later → ContentLoader card).
	void loadContent();

	void handleClientConnect(const common::ID clientId,
							 const common::ClientConnectMessage& connect,
							 std::uint64_t sequence);
	void handleClientInput(const common::ID clientId,
						   const common::ClientInput& clientInput,
						   std::uint64_t sequence);
	void enqueueWorldState(const common::ID sessionId, std::uint32_t tick, std::uint64_t sequence);
	int runLoop();

	net::InboundQueue m_inbound;
	net::OutboundQueue m_outbound;
	net::TcpServer m_server;
	// Order: map catalog before sessions (sessions hold a ref to the catalog).
	GameMapPool m_gameMapPool;
	GameSessionsPool m_gameSessionsPool;
	std::unordered_map<common::ID, ClientInputState> m_inputStates;
	std::uint32_t m_simTick = 0;
	bool m_running = false;
};

}  // namespace botf
