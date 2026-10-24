#pragma once

#include "common/types.hpp"
#include "engine/game_session.h"

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

namespace botf {

// Pool of live GameSession instances. Owned by ServerEngine.
// Owns client→session membership. Card 46: ensureDefaultFor → one shared session.
class GameSessionsPool {
public:
	GameSessionsPool() = default;

	GameSessionsPool(const GameSessionsPool&) = delete;
	GameSessionsPool& operator=(const GameSessionsPool&) = delete;

	// Client → session. If unbound: join the existing default session, or create one.
	GameSession& ensureDefaultFor(common::ClientID clientId);

	[[nodiscard]] GameSession* findGameSessionByClient(common::ClientID clientId);
	[[nodiscard]] const GameSession* findGameSessionByClient(common::ClientID clientId) const;

	// Drop client→session membership and destroy bound figure in that session's GameLogic.
	void removeClient(common::ClientID clientId);

	// Clients currently mapped to this session (for WorldState fan-out).
	[[nodiscard]] std::vector<common::ClientID> clientsOf(const GameSession& session) const;

	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] bool empty() const;

	// Iterate sessions (tick / broadcast).
	[[nodiscard]] const std::vector<std::unique_ptr<GameSession>>& sessions() const;
	[[nodiscard]] std::vector<std::unique_ptr<GameSession>>& sessions();

private:
	GameSession& createSession();

	std::vector<std::unique_ptr<GameSession>> m_sessions;
	std::unordered_map<common::ClientID, GameSession*> m_clientToSession;
};

}  // namespace botf
