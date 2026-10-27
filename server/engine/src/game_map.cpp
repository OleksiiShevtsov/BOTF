#include "engine/game_map.h"

#include "utils/map_loader.hpp"

#include <stdexcept>

botf::GameMap botf::GameMap::load(const common::ID mapId) {
	const auto data = utils::loadMap(mapId);
	if (!data) {
		throw std::runtime_error("GameMap::load: failed to load map id " + std::to_string(mapId));
	}
	GameMap map;
	map.m_id = data->id;
	map.m_name = data->name;
	map.m_width = data->width;
	map.m_height = data->height;
	map.m_spawns = data->spawns;
	return map;
}

botf::common::ID botf::GameMap::getId() const {
	return m_id;
}

const std::string& botf::GameMap::getName() const {
	return m_name;
}

float botf::GameMap::getWidth() const {
	return m_width;
}

float botf::GameMap::getHeight() const {
	return m_height;
}

const std::vector<botf::common::Point>& botf::GameMap::getSpawns() const {
	return m_spawns;
}
