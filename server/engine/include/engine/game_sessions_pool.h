#pragma once

#include "common/types.hpp"
#include "engine/game_map_pool.h"
#include "engine/game_session.h"
#include "engine/player.h"

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

namespace botf {

// Pool of live GameSession instances. Owned by ServerEngine.
// Seats hold shared_ptr<Player>; this pool keeps clientId → sessionId routing.
class GameSessionsPool {
public:
	explicit GameSessionsPool(GameMapPool& gameMapPool);

	GameSessionsPool(const GameSessionsPool&) = delete;
	GameSessionsPool& operator=(const GameSessionsPool&) = delete;

	// JoinSession: first Forming with free seats, else create (map picked now).
	common::ID attachPlayerToSession(std::shared_ptr<Player> player);

	[[nodiscard]] GameSession* findGameSessionByClientId(const common::ID clientId);
	[[nodiscard]] const GameSession* findGameSessionByClientId(const common::ID clientId) const;

	[[nodiscard]] GameSession* findGameSessionBySessionId(const common::ID sessionId);
	[[nodiscard]] const GameSession* findGameSessionBySessionId(const common::ID sessionId) const;

	void removePlayer(const common::ID clientId);

	[[nodiscard]] std::vector<common::ID> getSessionClients(const common::ID sessionId) const;

	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] bool empty() const;

	[[nodiscard]] const std::unordered_map<common::ID, std::unique_ptr<GameSession>>&
	sessions() const;
	[[nodiscard]] std::unordered_map<common::ID, std::unique_ptr<GameSession>>& sessions();

private:
	GameSession& createSession();

	GameMapPool& m_gameMapPool;
	std::unordered_map<common::ID, std::unique_ptr<GameSession>> m_sessions;// ???
	std::unordered_map<common::ID, common::ID> m_clientToSessionId;// ???
	common::ID m_nextId{1};
};

}  // namespace botf
