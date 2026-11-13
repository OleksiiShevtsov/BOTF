#pragma once

#include "common/dto.hpp"
#include "engine/bootstrap_storage.h"
#include "engine/game_sessions_pool.h"
#include "engine/player.h"
#include "net/tcp_server.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace botf {

// Server runtime: TCP + Players + BootstrapStorage + GameSessionsPool tick + WorldState.
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
	[[nodiscard]] BootstrapStorage& getBootstrapStorage();
	[[nodiscard]] const BootstrapStorage& getBootstrapStorage() const;
	[[nodiscard]] GameSessionsPool& getGameSessionsPool();
	[[nodiscard]] const GameSessionsPool& getGameSessionsPool() const;
	[[nodiscard]] std::uint32_t simTick() const;

private:
	void loadObjectData();

	[[nodiscard]] std::shared_ptr<Player> ensurePlayer(common::ID clientId);
	[[nodiscard]] std::shared_ptr<Player> findPlayerById(common::ID clientId) const;
	void removePlayerById(common::ID clientId);

	void handleClientConnect(const common::ID clientId,
							 const common::ClientConnectMessage& connect,
							 std::int64_t sequence);
	void handleJoinSession(const common::ID clientId,
						   const common::JoinSessionMessage& join,
						   std::int64_t sequence);
	void handleClientInput(const common::ID clientId, const common::ClientInput& clientInput);
	void tryStartSession(const common::ID sessionId, std::int64_t sequence);
	void enqueueWorldState(const common::ID sessionId, std::uint32_t tick, std::int64_t sequence);
	int runLoop();

	net::InboundQueue m_inbound;
	net::OutboundQueue m_outbound;
	net::TcpServer m_server;
	std::shared_ptr<BootstrapStorage> m_bootstrapStorage;
	GameSessionsPool m_gameSessionsPool;
	std::vector<std::shared_ptr<Player>> m_players;
	std::uint32_t m_simTick = 0;
	bool m_running = false;
};

}  // namespace botf
