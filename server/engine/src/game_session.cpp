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
