#pragma once

#include "common/life_status.hpp"
#include "common/types.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace botf::common {

// Client → server combined input for one tick (wire: ClientInput).
struct ClientInput {
	int dx{};
	int dy{};
	Point aim{};
	bool fire{};
	std::uint32_t tick{};
};

// Hello / Hello-ack (login-role). objectId == 0 on client request.
struct HelloMessage {
	std::string playerName;
	ObjectID objectId{};
};

// Pose of a controllable actor in a world snapshot.
struct PlayerPose {
	ObjectID id{};
	Point position{};
	Angle angle{};
	int hp{};
	LifeStatus lifeStatus{LifeStatus::Alive};
};

// Reserved for future bullets (mapped from proto when present).
struct BulletPose {
	ObjectID id{};
	ObjectID ownerId{};
	Point position{};
	Point velocity{};
};

// Authoritative world frame shared across logic ↔ protocol ↔ app.
struct WorldSnapshot {
	std::uint32_t tick{};
	std::vector<PlayerPose> players;
	std::vector<BulletPose> bullets;
};

}  // namespace botf::common
