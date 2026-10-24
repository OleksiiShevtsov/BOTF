#include "engine/game_session.h"

botf::GameSession::GameSession(common::SessionID id) : m_id{id} {}

botf::common::SessionID botf::GameSession::getId() const {
	return m_id;
}

botf::common::MapID botf::GameSession::getMapId() const {
	return m_mapId;
}

botf::GameLogic& botf::GameSession::getGameLogic() {
	return m_gameLogic;
}

const botf::GameLogic& botf::GameSession::getGameLogic() const {
	return m_gameLogic;
}
