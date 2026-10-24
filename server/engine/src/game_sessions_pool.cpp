#include "engine/game_sessions_pool.h"

botf::GameSession& botf::GameSessionsPool::ensureDefaultFor(common::ClientID clientId) {
	if (GameSession* existing = findByClient(clientId)) {
		return *existing;
	}

	GameSession* session = nullptr;
	if (!m_sessions.empty()) {
		// Card 46: one shared match for all clients.
		session = m_sessions.front().get();
	} else {
		session = &createSession(kDefaultMapId);
	}

	session->addClient(clientId);
	m_clientToSession[clientId] = session;
	return *session;
}

botf::GameSession* botf::GameSessionsPool::findByClient(common::ClientID clientId) {
	const auto it = m_clientToSession.find(clientId);
	if (it == m_clientToSession.end()) {
		return nullptr;
	}
	return it->second;
}

const botf::GameSession* botf::GameSessionsPool::findByClient(common::ClientID clientId) const {
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
	session->removeClient(clientId);
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

botf::GameSession& botf::GameSessionsPool::createSession(common::MapID mapId) {
	m_sessions.push_back(std::make_unique<GameSession>(mapId));
	return *m_sessions.back();
}
