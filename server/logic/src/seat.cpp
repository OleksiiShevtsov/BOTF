#include "logic/seat.h"

#include "engine/player.h"

#include <utility>

botf::Seat::Seat(const std::size_t index) : m_index{index} {}

std::size_t botf::Seat::getIndex() const {
	return m_index;
}

void botf::Seat::setIndex(const std::size_t index) {
	m_index = index;
}

bool botf::Seat::isFree() const {
	return m_player == nullptr;
}

std::shared_ptr<botf::Player> botf::Seat::getPlayer() const {
	return m_player;
}

void botf::Seat::setPlayer(std::shared_ptr<Player> player) {
	m_player = std::move(player);
}

void botf::Seat::clearPlayer() {
	m_player.reset();
}
