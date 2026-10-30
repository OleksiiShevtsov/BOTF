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
// Membership clientId → sessionId lives only here (card 46/47).
// Map templates: GameMapPool ref for createSession().
class GameSessionsPool {
public:
	explicit GameSessionsPool(GameMapPool& gameMapPool);

	GameSessionsPool(const GameSessionsPool&) = delete;
	GameSessionsPool& operator=(const GameSessionsPool&) = delete;

	// JoinSession path: first Forming with free seats, else create (map picked now).
	// Idempotent if client already mapped. Does not spawn figures (Forming).
	common::ID joinFormingOrCreate(const common::ID clientId);

	[[nodiscard]] GameSession* findGameSessionByClientId(const common::ID clientId);
	[[nodiscard]] const GameSession* findGameSessionByClientId(const common::ID clientId) const;

	[[nodiscard]] GameSession* findGameSessionBySessionId(const common::ID sessionId);
	[[nodiscard]] const GameSession* findGameSessionBySessionId(const common::ID sessionId) const;

	void removeClient(const common::ID clientId);

	[[nodiscard]] std::vector<common::ID> getSessionClients(const common::ID sessionId) const;

	[[nodiscard]] std::size_t size() const;
	[[nodiscard]] bool empty() const;

	[[nodiscard]] const std::unordered_map<common::ID, std::unique_ptr<GameSession>>&
	sessions() const;
	[[nodiscard]] std::unordered_map<common::ID, std::unique_ptr<GameSession>>& sessions();

private:
	GameSession& createSession();

	GameMapPool& m_gameMapPool;
	std::unordered_map<common::ID, std::unique_ptr<GameSession>> m_sessions;
	std::unordered_map<common::ID, common::ID> m_clientToSessionId;
	// TODO: нет БД — инкремент id сессии должен выдаваться из БД, не локальный счётчик.
	common::ID m_nextId{1};
};

}  // namespace botf
