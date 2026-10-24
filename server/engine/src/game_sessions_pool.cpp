#include "engine/game_sessions_pool.h"

botf::GameSession& botf::GameSessionsPool::ensureDefaultFor(common::ClientID clientId) {
	if (GameSession* existing = findGameSessionByClient(clientId)) {
		return *existing;
	}

	GameSession* session = nullptr;
	if (!m_sessions.empty()) {
		// Card 46: one shared match for all clients.
		session = m_sessions.front().get();
	} else {
		session = &createSession();
	}

	m_clientToSession[clientId] = session;
	return *session;
}

botf::GameSession* botf::GameSessionsPool::findGameSessionByClient(common::ClientID clientId) {
	const auto it = m_clientToSession.find(clientId);
	if (it == m_clientToSession.end()) {
		return nullptr;
	}
	return it->second;
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionByClient(
	common::ClientID clientId) const {
	const auto it = m_clientToSession.find(clientId);
	if (it == m_clientToSession.end()) {
		return nullptr;
	}
	return it->second;
}

void botf::GameSessionsPool::removeClient(common::ClientID clientId) {
	const auto it = m_clientToSession.find(clientId);
	if (it == m_clientToSession.end()) {
		return;
	}
	GameSession* session = it->second;
	m_clientToSession.erase(it);
	session->getGameLogic().removeClient(clientId);
}

std::vector<botf::common::ClientID> botf::GameSessionsPool::clientsOf(
	const GameSession& session) const {
	std::vector<common::ClientID> clients;
	for (const auto& [clientId, mapped] : m_clientToSession) {
		if (mapped == &session) {
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

const std::vector<std::unique_ptr<botf::GameSession>>& botf::GameSessionsPool::sessions() const {
	return m_sessions;
}

std::vector<std::unique_ptr<botf::GameSession>>& botf::GameSessionsPool::sessions() {
	return m_sessions;
}

botf::GameSession& botf::GameSessionsPool::createSession() {
	m_sessions.push_back(std::make_unique<GameSession>());
	return *m_sessions.back();
}
