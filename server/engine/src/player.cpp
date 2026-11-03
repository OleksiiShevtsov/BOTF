#include "engine/player.h"

#include <utility>

botf::Player::Player(const common::ID id) : m_id{id} {}

botf::common::ID botf::Player::getId() const {
	return m_id;
}

void botf::Player::setId(const common::ID id) {
	m_id = id;
}

const std::string& botf::Player::getName() const {
	return m_name;
}

void botf::Player::setName(std::string name) {
	m_name = std::move(name);
}
