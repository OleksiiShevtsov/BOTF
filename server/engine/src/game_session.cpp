#include "engine/game_session.h"

botf::GameSession::GameSession(common::MapID mapId) : m_mapId{mapId} {}

botf::common::MapID botf::GameSession::mapId() const {
	return m_mapId;
}

botf::GameLogic& botf::GameSession::logic() {
	return m_logic;
}

const botf::GameLogic& botf::GameSession::logic() const {
	return m_logic;
}

void botf::GameSession::addClient(common::ClientID clientId) {
	m_clients.insert(clientId);
}

void botf::GameSession::removeClient(common::ClientID clientId) {
	m_clients.erase(clientId);
	m_logic.removeClient(clientId);
}

bool botf::GameSession::contains(common::ClientID clientId) const {
	return m_clients.contains(clientId);
}

const std::unordered_set<botf::common::ClientID>& botf::GameSession::clients() const {
	return m_clients;
}
