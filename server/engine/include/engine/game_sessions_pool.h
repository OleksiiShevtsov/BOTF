#pragma once

#include "common/types.hpp"
#include "engine/game_session.h"
#include "engine/player.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace botf {

// Pool of live GameSession instances. Owned by ServerEngine.
// Client → session is resolved via seats (Player id), not a side index.
class GameSessionsPool {
public:
	GameSessionsPool() = default;

	GameSessionsPool(const GameSessionsPool&) = delete;
	GameSessionsPool& operator=(const GameSessionsPool&) = delete;

	// JoinSession: first Forming with free seats, else create (map assigned at beginPlaying).
	common::ID attachPlayerToSession(std::shared_ptr<Player> player, common::ID figureTypeId);

	[[nodiscard]] GameSession* findGameSessionByClientId(const common::ID clientId);
	[[nodiscard]] const GameSession* findGameSessionByClientId(const common::ID clientId) const;

	[[nodiscard]] GameSession* findGameSessionBySessionId(const common::ID sessionId);
	[[nodiscard]] const GameSession* findGameSessionBySessionId(const common::ID sessionId) const;

	void removePlayer(const common::ID clientId);

	[[nodiscard]] std::vector<common::ID> getSessionClients(const common::ID sessionId) const;

	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] bool empty() const;

	[[nodiscard]] const std::vector<std::unique_ptr<GameSession>>& sessions() const;
	[[nodiscard]] std::vector<std::unique_ptr<GameSession>>& sessions();

private:
	GameSession& createSession();

	std::vector<std::unique_ptr<GameSession>> m_sessions;
	common::ID m_nextId{1};
};

}  // namespace botf
