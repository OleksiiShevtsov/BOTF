#include "logic/game_map.h"

#include <utility>

botf::GameMap::GameMap(utils::MapData data)
	: m_id{data.id},
	  m_name{std::move(data.name)},
	  m_width{data.width},
	  m_height{data.height},
	  m_spawns{std::move(data.spawns)} {}

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
