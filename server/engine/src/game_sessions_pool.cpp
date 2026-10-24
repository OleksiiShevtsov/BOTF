#include "engine/game_sessions_pool.h"

botf::GameSession& botf::GameSessionsPool::ensureDefaultFor(common::ID clientId) {
	if (GameSession* existing = findGameSessionByClientId(clientId)) {
		return *existing;
	}

	common::ID sessionId{};
	if (!m_sessions.empty()) {
		// Card 46: one shared match for all clients.
		sessionId = m_sessions.begin()->first;
	} else {
		sessionId = createSession().getId();
	}

	m_clientToSessionId[clientId] = sessionId;
	return *m_sessions.at(sessionId);
}

botf::GameSession* botf::GameSessionsPool::findGameSessionByClientId(common::ID clientId) {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return nullptr;
	}
	return findGameSessionBySessionId(it->second);
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionByClientId(
	common::ID clientId) const {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return nullptr;
	}
	return findGameSessionBySessionId(it->second);
}

botf::GameSession* botf::GameSessionsPool::findGameSessionBySessionId(common::ID sessionId) {
	const auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return nullptr;
	}
	return it->second.get();
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionBySessionId(
	common::ID sessionId) const {
	const auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return nullptr;
	}
	return it->second.get();
}

void botf::GameSessionsPool::removeClient(common::ID clientId) {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return;
	}
	const common::ID sessionId = it->second;
	m_clientToSessionId.erase(it);
	if (GameSession* session = findGameSessionBySessionId(sessionId)) {
		session->getGameLogic().removeClient(clientId);
	}
}

std::vector<botf::common::ID> botf::GameSessionsPool::getSessionClients(
	common::ID sessionId) const {
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
	const common::ID id = m_nextId;
	++m_nextId;
	auto [it, inserted] = m_sessions.emplace(id, std::make_unique<GameSession>(id));
	(void)inserted;
	return *it->second;
}
