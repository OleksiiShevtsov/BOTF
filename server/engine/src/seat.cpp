#include "engine/seat.h"

#include <utility>

botf::Seat::Seat(const common::IDX index) : m_index{index} {}

botf::common::IDX botf::Seat::getIndex() const {
	return m_index;
}

void botf::Seat::setIndex(const common::IDX index) {
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

botf::common::ID botf::Seat::getFigureTypeId() const {
	return m_figureTypeId;
}

void botf::Seat::setFigureTypeId(const common::ID figureTypeId) {
	m_figureTypeId = figureTypeId;
}
