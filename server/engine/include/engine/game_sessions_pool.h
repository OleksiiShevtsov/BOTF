#pragma once

#include "common/types.hpp"
#include "engine/game_map_pool.h"
#include "engine/game_session.h"

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

namespace botf {

// Pool of live GameSession instances. Owned by ServerEngine.
// Membership: ID → ID → GameSession. Card 46: one shared session.
// Map templates: GameMapPool (card 51).
class GameSessionsPool {
public:
	GameSessionsPool() = default;

	GameSessionsPool(const GameSessionsPool&) = delete;
	GameSessionsPool& operator=(const GameSessionsPool&) = delete;

	// Client → session. If unbound: join the existing default session, or create one.
	// Returns session id (not a reference).
	common::ID ensureDefaultFor(const common::ID clientId);

	[[nodiscard]] GameSession* findGameSessionByClientId(const common::ID clientId);
	[[nodiscard]] const GameSession* findGameSessionByClientId(const common::ID clientId) const;

	[[nodiscard]] GameSession* findGameSessionBySessionId(const common::ID sessionId);
	[[nodiscard]] const GameSession* findGameSessionBySessionId(const common::ID sessionId) const;

	// Drop client→session membership and destroy bound figure in that session.
	void removeClient(const common::ID clientId);

	// Clients currently mapped to this session (for WorldState fan-out).
	[[nodiscard]] std::vector<common::ID> getSessionClients(const common::ID sessionId) const;

	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] bool empty() const;

	[[nodiscard]] GameMapPool& getGameMapPool();
	[[nodiscard]] const GameMapPool& getGameMapPool() const;

	[[nodiscard]] const std::unordered_map<common::ID, std::unique_ptr<GameSession>>&
	sessions() const;
	[[nodiscard]] std::unordered_map<common::ID, std::unique_ptr<GameSession>>& sessions();

private:
	GameSession& createSession();
	void ensureMapsLoaded();

	GameMapPool m_gameMapPool;
	std::unordered_map<common::ID, std::unique_ptr<GameSession>> m_sessions;
	std::unordered_map<common::ID, common::ID> m_clientToSessionId;
	// TODO: нет БД — инкремент id сессии должен выдаваться из БД, не локальный счётчик.
	common::ID m_nextId{1};
};

}  // namespace botf
