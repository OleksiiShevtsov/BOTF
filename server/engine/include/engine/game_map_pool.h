#pragma once

#include "common/types.hpp"
#include "logic/game_map.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace botf {

// Catalog of map templates (card 51). Not live sessions — see GameSessionsPool.
// Owns shared templates; sessions hold shared_ptr<const GameMap>.
class GameMapPool {
public:
	GameMapPool() = default;

	GameMapPool(const GameMapPool&) = delete;
	GameMapPool& operator=(const GameMapPool&) = delete;

	void clear();

	// Scan content/maps, load each GameMap into cache. Returns false if no maps.
	bool refresh();

	[[nodiscard]] bool empty() const;
	[[nodiscard]] std::size_t size() const;

	// Uniform random template from the cache.
	[[nodiscard]] std::shared_ptr<const GameMap> pickRandomGameMap() const;

	[[nodiscard]] std::shared_ptr<const GameMap> findGameMapById(const common::ID mapId) const;
	[[nodiscard]] std::shared_ptr<const GameMap> getGameMapById(const common::ID mapId) const;

private:
	[[nodiscard]] std::shared_ptr<const GameMap> loadGameMap(const common::ID mapId) const;

	std::vector<std::shared_ptr<const GameMap>> m_gameMaps;
};

}  // namespace botf
