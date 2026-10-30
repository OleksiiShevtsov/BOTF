#include "engine/game_sessions_pool.h"

#include "common/log.hpp"

#include <memory>
#include <stdexcept>

botf::GameSessionsPool::GameSessionsPool(GameMapPool& gameMapPool)
	: m_gameMapPool(gameMapPool) {}

botf::common::ID botf::GameSessionsPool::joinFormingOrCreate(const common::ID clientId) {
	if (GameSession* existing = findGameSessionByClientId(clientId)) {
		return existing->getId();
	}

	for (auto& [sessionId, gameSession] : m_sessions) {
		const std::size_t occupied = getSessionClients(sessionId).size();
		if (gameSession->hasFreeSeats(occupied)) {
			m_clientToSessionId[clientId] = sessionId;
			botf::log::info("[server] client {} joined Forming session {} ({}/{})", clientId,
							sessionId, occupied + 1, gameSession->seatCapacity());
			return sessionId;
		}
	}

	GameSession& created = createSession();
	m_clientToSessionId[clientId] = created.getId();
	botf::log::info("[server] client {} created Forming session {} (1/{}, mapId={})", clientId,
					created.getId(), created.seatCapacity(), created.getGameMap().getId());
	return created.getId();
}

botf::GameSession* botf::GameSessionsPool::findGameSessionByClientId(const common::ID clientId) {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return nullptr;
	}
	return findGameSessionBySessionId(it->second);
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionByClientId(
	const common::ID clientId) const {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return nullptr;
	}
	return findGameSessionBySessionId(it->second);
}

botf::GameSession* botf::GameSessionsPool::findGameSessionBySessionId(const common::ID sessionId) {
	const auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return nullptr;
	}
	return it->second.get();
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionBySessionId(
	const common::ID sessionId) const {
	const auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return nullptr;
	}
	return it->second.get();
}

void botf::GameSessionsPool::removeClient(const common::ID clientId) {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return;
	}
	const common::ID sessionId = it->second;
	m_clientToSessionId.erase(it);
	if (GameSession* gameSession = findGameSessionBySessionId(sessionId)) {
		gameSession->removeClient(clientId);
	}
}

std::vector<botf::common::ID> botf::GameSessionsPool::getSessionClients(
	const common::ID sessionId) const {
	std::vector<common::ID> clients;
	for (const auto& [clientId, mappedId] : m_clientToSessionId) {
		if (mappedId == sessionId) {
			clients.push_back(clientId);
		}
	}
	return clients;
}

std::size_t botf::GameSessionsPool::size() const {
	return m_sessions.size();
}

bool botf::GameSessionsPool::empty() const {
	return m_sessions.empty();
}

const std::unordered_map<botf::common::ID, std::unique_ptr<botf::GameSession>>&
botf::GameSessionsPool::sessions() const {
	return m_sessions;
}

std::unordered_map<botf::common::ID, std::unique_ptr<botf::GameSession>>&
botf::GameSessionsPool::sessions() {
	return m_sessions;
}

botf::GameSession& botf::GameSessionsPool::createSession() {
	if (m_gameMapPool.empty()) {
		throw std::runtime_error(
			"GameSessionsPool::createSession: GameMapPool empty (ServerEngine must loadObjectData)");
	}
	const std::shared_ptr<const GameMap> gameMap = m_gameMapPool.pickRandomGameMap();

	const common::ID id = m_nextId;
	++m_nextId;
	auto [it, inserted] =
		m_sessions.emplace(id, std::make_unique<GameSession>(id, gameMap));
	(void)inserted;
	return *it->second;
}
