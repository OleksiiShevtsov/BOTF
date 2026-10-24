#include "engine/game_session.h"

botf::GameSession::GameSession() = default;

botf::common::MapID botf::GameSession::getMapId() const {
	return m_mapId;
}

botf::GameLogic& botf::GameSession::getGameLogic() {
	return m_gameLogic;
}

const botf::GameLogic& botf::GameSession::getGameLogic() const {
	return m_gameLogic;
}

void botf::GameSession::addClient(common::ClientID clientId) {
	m_clients.insert(clientId);
}

void botf::GameSession::removeClient(common::ClientID clientId) {
	m_clients.erase(clientId);
	m_gameLogic.removeClient(clientId);
}

bool botf::GameSession::contains(common::ClientID clientId) const {
	return m_clients.contains(clientId);
}

const std::unordered_set<botf::common::ClientID>& botf::GameSession::clients() const {
	return m_clients;
}
