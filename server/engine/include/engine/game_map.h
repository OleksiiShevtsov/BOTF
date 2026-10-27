#pragma once

#include "common/types.hpp"

#include <string>
#include <vector>

namespace botf {

// Server-side map template (card 51). Immutable after load.
// Loads via utils (JSON under content/maps/<id>/). Not the client app::GameMap.
class GameMap {
public:
	GameMap() = default;

	GameMap(const common::ID id,
			std::string name,
			float width,
			float height,
			std::vector<common::Point> spawns);

	// Load from content via utils::loadMap. Throws std::runtime_error on failure.
	[[nodiscard]] static GameMap load(const common::ID mapId);

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] const std::string& getName() const;
	[[nodiscard]] float getWidth() const;
	[[nodiscard]] float getHeight() const;
	[[nodiscard]] const std::vector<common::Point>& getSpawns() const;

private:
	common::ID m_id{};
	std::string m_name;
	float m_width{0.f};
	float m_height{0.f};
	std::vector<common::Point> m_spawns;
};

}  // namespace botf
