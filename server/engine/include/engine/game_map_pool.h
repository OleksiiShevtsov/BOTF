#pragma once

#include "common/types.hpp"
#include "engine/game_map.h"

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace botf {

// Catalog of map templates (card 51). Not live sessions — see GameSessionsPool.
class GameMapPool {
public:
	GameMapPool() = default;

	GameMapPool(const GameMapPool&) = delete;
	GameMapPool& operator=(const GameMapPool&) = delete;

	// Scan content/maps, load each GameMap into cache. Returns false if no maps.
	bool refresh();

	[[nodiscard]] bool empty() const;
	[[nodiscard]] std::size_t size() const;

	[[nodiscard]] const std::vector<common::ID>& mapIds() const;

	// Round-robin across known ids (one map → always that id).
	[[nodiscard]] common::ID pickNextMapId();

	[[nodiscard]] const GameMap* findMap(const common::ID mapId) const;
	// Throws if missing — prefer after refresh / known id from pickNext.
	[[nodiscard]] const GameMap& getMap(const common::ID mapId) const;

private:
	std::vector<common::ID> m_mapIds;
	std::unordered_map<common::ID, GameMap> m_maps;
	std::size_t m_nextPickIndex{0};
};

}  // namespace botf
