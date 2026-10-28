#include "engine/game_map_pool.h"

#include "utils/map_loader.hpp"

#include <random>
#include <stdexcept>

void botf::GameMapPool::clear() {
	m_gameMaps.clear();
}

bool botf::GameMapPool::refresh() {
	clear();
	for (const common::ID mapId : utils::listMapIds()) {
		m_gameMaps.push_back(std::make_shared<const GameMap>(GameMap::load(mapId)));
	}
	return !m_gameMaps.empty();
}

bool botf::GameMapPool::empty() const {
	return m_gameMaps.empty();
}

std::size_t botf::GameMapPool::size() const {
	return m_gameMaps.size();
}

botf::common::ID botf::GameMapPool::pickRandomMapId() const {
	if (m_gameMaps.empty()) {
		throw std::runtime_error("GameMapPool::pickRandomMapId: pool is empty (call refresh)");
	}
	thread_local std::mt19937 rng{std::random_device{}()};
	std::uniform_int_distribution<std::size_t> dist{0, m_gameMaps.size() - 1};
	return m_gameMaps[dist(rng)]->getId();
}

std::shared_ptr<const botf::GameMap> botf::GameMapPool::findGameMapById(
	const common::ID mapId) const {
	for (const auto& gameMap : m_gameMaps) {
		if (gameMap && gameMap->getId() == mapId) {
			return gameMap;
		}
	}
	return nullptr;
}

std::shared_ptr<const botf::GameMap> botf::GameMapPool::getGameMapById(
	const common::ID mapId) const {
	std::shared_ptr<const GameMap> gameMap = findGameMapById(mapId);
	if (!gameMap) {
		throw std::runtime_error("GameMapPool::getGameMapById: unknown map id " +
								 std::to_string(mapId));
	}
	return gameMap;
}
