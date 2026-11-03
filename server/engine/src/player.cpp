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

const std::optional<botf::common::ClientInput>& botf::Player::lastClientInput() const {
	return m_lastClientInput;
}

void botf::Player::setLastClientInput(common::ClientInput input) {
	m_lastClientInput = std::move(input);
}
