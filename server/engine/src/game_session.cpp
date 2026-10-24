#include "engine/game_session.h"

botf::GameSession::GameSession(common::ID id) : m_id{id} {}

botf::common::ID botf::GameSession::getId() const {
	return m_id;
}

botf::GameLogic& botf::GameSession::getGameLogic() {
	return m_gameLogic;
}

const botf::GameLogic& botf::GameSession::getGameLogic() const {
	return m_gameLogic;
}
