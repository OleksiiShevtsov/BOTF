#include "engine/game_map.h"

#include "utils/map_loader.hpp"

#include <stdexcept>

botf::GameMap::GameMap(const common::ID id,
						std::string name,
						const float width,
						const float height,
						std::vector<common::Point> spawns)
	: m_id{id},
	  m_name{std::move(name)},
	  m_width{width},
	  m_height{height},
	  m_spawns{std::move(spawns)} {}

botf::GameMap botf::GameMap::load(const common::ID mapId) {
	const auto data = utils::loadMap(mapId);
	if (!data) {
		throw std::runtime_error("GameMap::load: failed to load map id " + std::to_string(mapId));
	}
	return GameMap{data->id, data->name, data->width, data->height, data->spawns};
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
