#include "engine/game_sessions_pool.h"

#include <memory>
#include <stdexcept>

void botf::GameSessionsPool::ensureMapsLoaded() {
	if (!m_gameMapPool.empty()) {
		return;
	}
	if (!m_gameMapPool.refresh()) {
		throw std::runtime_error("GameSessionsPool: no maps in content/maps (GameMapPool empty)");
	}
}

botf::common::ID botf::GameSessionsPool::ensureDefaultFor(const common::ID clientId) {
	if (GameSession* existing = findGameSessionByClientId(clientId)) {
		return existing->getId();
	}

	// TODO: пока нет распределителя клиентов по доступным сессиям —
	// всегда сажаем в первую (и пока единственную) сессию.
	const common::ID sessionId =
		!m_sessions.empty() ? m_sessions.begin()->first : createSession().getId();

	m_clientToSessionId[clientId] = sessionId;
	return sessionId;
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

botf::GameMapPool& botf::GameSessionsPool::getGameMapPool() {
	return m_gameMapPool;
}

const botf::GameMapPool& botf::GameSessionsPool::getGameMapPool() const {
	return m_gameMapPool;
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
	ensureMapsLoaded();
	const std::shared_ptr<const GameMap> gameMap = m_gameMapPool.pickRandomGameMap();

	const common::ID id = m_nextId;
	++m_nextId;
	auto [it, inserted] =
		m_sessions.emplace(id, std::make_unique<GameSession>(id, gameMap));
	(void)inserted;
	return *it->second;
}
