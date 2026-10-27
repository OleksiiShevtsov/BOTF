#pragma once

#include "common/types.hpp"

#include <string>
#include <vector>

namespace botf {

// Server-side map template (card 51). Immutable after load.
// Create only via load() — fields come from content JSON (grows over time: collision, …).
// Not the client app::GameMap.
class GameMap {
public:
	// Load from content via utils::loadMap. Throws std::runtime_error on failure.
	[[nodiscard]] static GameMap load(const common::ID mapId);

	GameMap(const GameMap&) = default;
	GameMap& operator=(const GameMap&) = default;
	GameMap(GameMap&&) = default;
	GameMap& operator=(GameMap&&) = default;

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] const std::string& getName() const;
	[[nodiscard]] float getWidth() const;
	[[nodiscard]] float getHeight() const;
	[[nodiscard]] const std::vector<common::Point>& getSpawns() const;

private:
	GameMap() = default;

	common::ID m_id{};
	std::string m_name;
	float m_width{0.f};
	float m_height{0.f};
	std::vector<common::Point> m_spawns;
};

}  // namespace botf
