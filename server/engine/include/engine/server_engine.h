#pragma once

#include "common/dto.hpp"
#include "engine/game_sessions_pool.h"
#include "net/tcp_server.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>

namespace botf {

// Server runtime: TCP + GameSessionsPool tick + WorldState broadcast.
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
	[[nodiscard]] GameSessionsPool& sessions();
	[[nodiscard]] const GameSessionsPool& sessions() const;
	[[nodiscard]] std::uint32_t simTick() const;
	[[nodiscard]] std::optional<common::ClientInput> lastClientInput(
		common::ClientID clientId) const;

private:
	struct ClientInputState {
		common::ClientInput command{};
		std::uint64_t sequence = 0;
	};

	common::ObjectID bindOrSpawnForClient(GameSession& session, common::ClientID clientId);
	void handleClientConnect(common::ClientID clientId,
							 const common::ClientConnectMessage& connect,
							 std::uint64_t sequence);
	void handleClientInput(common::ClientID clientId,
						   const common::ClientInput& input,
						   std::uint64_t sequence);
	void enqueueWorldState(const GameSession& session, std::uint32_t tick, std::uint64_t sequence);
	int runLoop();

	net::InboundQueue m_inbound;
	net::OutboundQueue m_outbound;
	net::TcpServer m_server;
	GameSessionsPool m_gameSessionsPool;
	std::unordered_map<common::ClientID, ClientInputState> m_inputStates;
	std::uint32_t m_simTick = 0;
	bool m_running = false;
};

}  // namespace botf
