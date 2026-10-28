#pragma once

#include "common/types.hpp"
#include "utils/map_loader.hpp"

#include <string>
#include <vector>

namespace botf {

// Server-side map template (card 51). Immutable after construction from MapData.
// Seat capacity = getSpawns().size(). Not the client app::GameMap.
class GameMap {
public:
	explicit GameMap(utils::MapData data);

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
	common::ID m_id{};
	std::string m_name;
	float m_width{0.f};
	float m_height{0.f};
	std::vector<common::Point> m_spawns;
};

}  // namespace botf
