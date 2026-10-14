#pragma once

#include "common/types.hpp"

#include <cstdint>
#include <vector>

namespace botf::common {

// Client → server movement intent for one tick.
struct MoveCommand {
	int dx{};
	int dy{};
	std::uint32_t tick{};
};

// Pose of a controllable actor in a world snapshot.
struct PlayerPose {
	ID id{};
	Point position{};
	Angle angle{};
};

// Reserved for future projectiles (mapped from proto when present).
struct ProjectilePose {
	ID id{};
	ID ownerId{};
	Point position{};
	Point velocity{};
};

// Authoritative world frame shared across logic ↔ protocol ↔ app.
struct WorldSnapshot {
	std::uint32_t tick{};
	std::vector<PlayerPose> players;
	std::vector<ProjectilePose> projectiles;
};

}  // namespace botf::common
