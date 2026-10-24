#pragma once

#include "common/types.hpp"
#include "engine/game_session.h"

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

namespace botf {

// Pool of live GameSession instances. Owned by ServerEngine.
// Membership: ID → ID → GameSession. Card 46: one shared session.
class GameSessionsPool {
public:
	GameSessionsPool() = default;

	GameSessionsPool(const GameSessionsPool&) = delete;
	GameSessionsPool& operator=(const GameSessionsPool&) = delete;

	// Client → session. If unbound: join the existing default session, or create one.
	GameSession& ensureDefaultFor(common::ID clientId);

	[[nodiscard]] GameSession* findGameSessionByClient(common::ID clientId);
	[[nodiscard]] const GameSession* findGameSessionByClient(common::ID clientId) const;

	[[nodiscard]] GameSession* findGameSessionById(common::ID sessionId);
	[[nodiscard]] const GameSession* findGameSessionById(common::ID sessionId) const;

	// Drop client→session membership and destroy bound figure in that session's GameLogic.
	void removeClient(common::ID clientId);

	// Clients currently mapped to this session (for WorldState fan-out).
	[[nodiscard]] std::vector<common::ID> clientsOf(const GameSession& session) const;

	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] bool empty() const;

	[[nodiscard]] const std::unordered_map<common::ID, std::unique_ptr<GameSession>>&
	sessions() const;
	[[nodiscard]] std::unordered_map<common::ID, std::unique_ptr<GameSession>>& sessions();

private:
	GameSession& createSession();

	std::unordered_map<common::ID, std::unique_ptr<GameSession>> m_sessions;
	std::unordered_map<common::ID, common::ID> m_clientToSessionId;
	common::ID m_nextId{1};
};

}  // namespace botf
