#include "engine/game_sessions_pool.h"

botf::GameSession& botf::GameSessionsPool::ensureDefaultFor(common::ClientID clientId) {
	if (GameSession* existing = findGameSessionByClient(clientId)) {
		return *existing;
	}

	common::SessionID sessionId{};
	if (!m_sessions.empty()) {
		// Card 46: one shared match for all clients.
		sessionId = m_sessions.begin()->first;
	} else {
		sessionId = createSession().getId();
	}

	m_clientToSessionId[clientId] = sessionId;
	return *m_sessions.at(sessionId);
}

botf::GameSession* botf::GameSessionsPool::findGameSessionByClient(common::ClientID clientId) {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return nullptr;
	}
	return findGameSessionById(it->second);
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionByClient(
	common::ClientID clientId) const {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return nullptr;
	}
	return findGameSessionById(it->second);
}

botf::GameSession* botf::GameSessionsPool::findGameSessionById(common::SessionID sessionId) {
	const auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return nullptr;
	}
	return it->second.get();
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionById(
	common::SessionID sessionId) const {
	const auto it = m_sessions.find(sessionId);
	if (it == m_sessions.end()) {
		return nullptr;
	}
	return it->second.get();
}

void botf::GameSessionsPool::removeClient(common::ClientID clientId) {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return;
	}
	const common::SessionID sessionId = it->second;
	m_clientToSessionId.erase(it);
	if (GameSession* session = findGameSessionById(sessionId)) {
		session->getGameLogic().removeClient(clientId);
	}
}

std::vector<botf::common::ClientID> botf::GameSessionsPool::clientsOf(
	const GameSession& session) const {
	std::vector<common::ClientID> clients;
	const common::SessionID sessionId = session.getId();
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

const std::unordered_map<botf::common::SessionID, std::unique_ptr<botf::GameSession>>&
botf::GameSessionsPool::sessions() const {
	return m_sessions;
}

std::unordered_map<botf::common::SessionID, std::unique_ptr<botf::GameSession>>&
botf::GameSessionsPool::sessions() {
	return m_sessions;
}

botf::GameSession& botf::GameSessionsPool::createSession() {
	const common::SessionID id = m_nextSessionId;
	++m_nextSessionId;
	auto [it, inserted] = m_sessions.emplace(id, std::make_unique<GameSession>(id));
	(void)inserted;
	return *it->second;
}
