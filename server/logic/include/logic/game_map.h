#pragma once

#include "common/types.hpp"

#include <string>
#include <vector>

namespace botf {

class GameMapPool;

// Server-side map template (card 51). Immutable after pool load.
// Fields come from content JSON (grows over time: collision, …).
// Seat capacity = getSpawns().size(). Not the client app::GameMap.
// Created only by GameMapPool (friend).
class GameMap {
	friend class GameMapPool;

public:
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
