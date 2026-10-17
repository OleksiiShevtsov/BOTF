#include "app/player.h"

botf::app::Player::Player() : m_id{0} {}

botf::app::Player::Player(common::ObjectID id) : m_id{id} {}

botf::common::ObjectID botf::app::Player::getId() const {
	return m_id;
}

void botf::app::Player::setId(common::ObjectID id) {
	m_id = id;
}
