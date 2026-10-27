#include "engine/game_map_pool.h"

#include "utils/map_loader.hpp"

#include <stdexcept>

bool botf::GameMapPool::refresh() {
	m_mapIds.clear();
	m_maps.clear();
	m_nextPickIndex = 0;

	m_mapIds = utils::listMapIds();
	for (const common::ID mapId : m_mapIds) {
		m_maps.emplace(mapId, GameMap::load(mapId));
	}
	return !m_maps.empty();
}

bool botf::GameMapPool::empty() const {
	return m_maps.empty();
}

std::size_t botf::GameMapPool::size() const {
	return m_maps.size();
}

const std::vector<botf::common::ID>& botf::GameMapPool::mapIds() const {
	return m_mapIds;
}

botf::common::ID botf::GameMapPool::pickNextMapId() {
	if (m_mapIds.empty()) {
		throw std::runtime_error("GameMapPool::pickNextMapId: pool is empty (call refresh)");
	}
	const common::ID id = m_mapIds[m_nextPickIndex % m_mapIds.size()];
	++m_nextPickIndex;
	return id;
}

const botf::GameMap* botf::GameMapPool::findMap(const common::ID mapId) const {
	const auto it = m_maps.find(mapId);
	if (it == m_maps.end()) {
		return nullptr;
	}
	return &it->second;
}

const botf::GameMap& botf::GameMapPool::getMap(const common::ID mapId) const {
	const GameMap* map = findMap(mapId);
	if (!map) {
		throw std::runtime_error("GameMapPool::getMap: unknown map id " + std::to_string(mapId));
	}
	return *map;
}
