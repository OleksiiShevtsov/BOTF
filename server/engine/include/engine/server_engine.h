#pragma once

#include "common/dto.hpp"
#include "engine/game_map_pool.h"
#include "engine/game_sessions_pool.h"
#include "engine/player.h"
#include "net/tcp_server.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace botf {

// Server runtime: TCP + Players + catalogs + GameSessionsPool tick + WorldState.
class ServerEngine {
public:
	ServerEngine();
	~ServerEngine();

	ServerEngine(const ServerEngine&) = delete;
	ServerEngine& operator=(const ServerEngine&) = delete;

	int run(std::uint16_t port = 54545);

	void requestStop();

	bool startListen(std::uint16_t port);
	void shutdown();
	void pollNetwork();
	void applyInbound();
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
	void loadObjectData();

	[[nodiscard]] std::shared_ptr<Player> ensurePlayer(common::ID clientId);
	[[nodiscard]] std::shared_ptr<Player> findPlayer(common::ID clientId) const;
	void removePlayerById(common::ID clientId);

	void handleClientConnect(const common::ID clientId,
							 const common::ClientConnectMessage& connect,
							 std::int64_t sequence);
	void handleJoinSession(const common::ID clientId, std::int64_t sequence);
	void handleClientInput(const common::ID clientId,
						   const common::ClientInput& clientInput,
						   std::int64_t sequence);
	void tryStartSession(const common::ID sessionId, std::int64_t sequence);
	void enqueueWorldState(const common::ID sessionId, std::uint32_t tick, std::int64_t sequence);
	int runLoop();

	net::InboundQueue m_inbound;
	net::OutboundQueue m_outbound;
	net::TcpServer m_server;
	GameMapPool m_gameMapPool;
	GameSessionsPool m_gameSessionsPool;
	std::vector<std::shared_ptr<Player>> m_players;
	std::uint32_t m_simTick = 0;
	bool m_running = false;
};

}  // namespace botf
