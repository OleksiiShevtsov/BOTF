#pragma once

#include "common/types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace botf::utils {

// Plain data from content/maps/<id>/map.json (server slice + optional client path).
// No nlohmann in this header — parsed inside utils.
// Seat capacity = spawns.size() (no separate maxNumberPlayers).
struct MapData {
	common::ID id{};
	std::string name;
	float width{0.f};
	float height{0.f};
	std::vector<common::Point> spawns;
	std::string clientBackground;  // relative to map dir; may be empty
};

// Directory names under content/maps/ that parse as numeric ids.
[[nodiscard]] std::vector<common::ID> listMapIds();

// Load one map. nullopt if missing / invalid JSON / schema error.
[[nodiscard]] std::optional<MapData> loadMap(common::ID mapId);

}  // namespace botf::utils
