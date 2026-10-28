#include "engine/game_map_pool.h"

#include "utils/map_loader.hpp"

#include <random>
#include <stdexcept>

void botf::GameMapPool::clear() {
	m_gameMaps.clear();
}

std::shared_ptr<const botf::GameMap> botf::GameMapPool::loadGameMap(const common::ID mapId) const {
	const auto data = utils::loadMap(mapId);
	if (!data) {
		throw std::runtime_error("GameMapPool::loadGameMap: failed to load map id " +
								 std::to_string(mapId));
	}

	// GameMapPool is friend — private default ctor + field fill (no public load/ctor).
	std::shared_ptr<GameMap> gameMap{new GameMap()};
	gameMap->m_id = data->id;
	gameMap->m_name = data->name;
	gameMap->m_width = data->width;
	gameMap->m_height = data->height;
	gameMap->m_maxNumberPlayers = data->maxNumberPlayers;
	gameMap->m_spawns = data->spawns;
	return gameMap;
}

bool botf::GameMapPool::refresh() {
	clear();
	for (const common::ID mapId : utils::listMapIds()) {
		m_gameMaps.push_back(loadGameMap(mapId));
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
