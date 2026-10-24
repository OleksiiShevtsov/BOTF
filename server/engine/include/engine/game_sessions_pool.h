#pragma once

#include "common/types.hpp"
#include "engine/game_session.h"

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

namespace botf {

// Pool of live GameSession instances. Owned by ServerEngine.
// Card 46: ensureDefaultFor puts every client into one shared session (scaffold for 47).
class GameSessionsPool {
public:
	GameSessionsPool() = default;

	GameSessionsPool(const GameSessionsPool&) = delete;
	GameSessionsPool& operator=(const GameSessionsPool&) = delete;

	// Client → session. If unbound: join the existing default session, or create one.
	GameSession& ensureDefaultFor(common::ClientID clientId);

	[[nodiscard]] GameSession* findByClient(common::ClientID clientId);
	[[nodiscard]] const GameSession* findByClient(common::ClientID clientId) const;

	// Drop client from its session (and destroy bound figure via GameSession::removeClient).
	void removeClient(common::ClientID clientId);

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
