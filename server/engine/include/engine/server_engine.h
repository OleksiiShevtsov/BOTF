#pragma once

#include "common/dto.hpp"
#include "logic/game_logic.h"
#include "net/tcp_server.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>

namespace botf {

// Server runtime: TCP + GameLogic tick + WorldState broadcast.
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
	// Hello (spawn/bind + ack) and Move (own entity only).
	void applyInbound();
	// game.tick(dt) + enqueue WorldState for the new sim tick.
	void advanceSim(float dt);

	[[nodiscard]] std::uint16_t port() const;
	[[nodiscard]] std::size_t clientCount() const;
	[[nodiscard]] const GameLogic& game() const;
	[[nodiscard]] std::uint32_t simTick() const;
	[[nodiscard]] std::optional<common::MoveCommand> lastMove(common::ClientID clientId) const;

private:
	struct ClientInputState {
		common::MoveCommand command{};
		std::uint64_t sequence = 0;
	};

	common::ObjectID bindOrSpawnForClient(common::ClientID clientId);
	void handleHello(common::ClientID clientId,
					 const common::HelloMessage& hello,
					 std::uint64_t sequence);
	void handleMove(common::ClientID clientId,
					const common::MoveCommand& command,
					std::uint64_t sequence);
	void enqueueWorldState(std::uint32_t tick, std::uint64_t sequence);
	int runLoop();

	net::InboundQueue m_inbound;
	net::OutboundQueue m_outbound;
	net::TcpServer m_server;
	GameLogic m_game;
	std::unordered_map<common::ClientID, ClientInputState> m_inputStates;
	std::uint32_t m_simTick = 0;
	bool m_running = false;
};

}  // namespace botf
