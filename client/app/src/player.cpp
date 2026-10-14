#include "app/player.h"

botf::app::Player::Player() : m_id{0} {}

botf::app::Player::Player(common::ID id) : m_id{id} {}

botf::common::ID botf::app::Player::id() const {
	return m_id;
}

void botf::app::Player::setId(common::ID id) {
	m_id = id;
}
