#include "engine/game_map_pool.h"

#include "utils/map_loader.hpp"
#include "utils/random.hpp"

void botf::GameMapPool::clear() {
	m_gameMaps.clear();
}

bool botf::GameMapPool::load() {
	clear();
	for (const common::ID mapId : utils::listMapIds()) {
		const auto data = utils::loadMap(mapId);
		if (!data) {
			clear();
			return false;
		}
		m_gameMaps.push_back(std::make_shared<const GameMap>(*data));
	}
	return !m_gameMaps.empty();
}

bool botf::GameMapPool::empty() const {
	return m_gameMaps.empty();
}

std::size_t botf::GameMapPool::size() const {
	return m_gameMaps.size();
}

std::shared_ptr<const botf::GameMap> botf::GameMapPool::pickRandomGameMap() const {
	if (m_gameMaps.empty()) {
		return nullptr;
	}
	return m_gameMaps[utils::randomIndex(m_gameMaps.size())];
}

std::shared_ptr<const botf::GameMap> botf::GameMapPool::getGameMapById(
	const common::ID mapId) const {
	for (const auto& gameMap : m_gameMaps) {
		if (gameMap && gameMap->getId() == mapId) {
			return gameMap;
		}
	}
	return nullptr;
}
