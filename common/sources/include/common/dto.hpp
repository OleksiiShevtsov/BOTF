#pragma once

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

// ClientConnect / ack (login-role). boundObjectId == 0 and mapId == 0 on client request.
struct ClientConnectMessage {
	std::string playerName;
	ObjectID boundObjectId{};
	MapID mapId{};
};

// Pose of a controllable actor in a world snapshot.
struct FigurePose {
	ObjectID objectId{};
	Point position{};
	Angle aimAngle{};
	int hitPoints{};
	LifeStatus lifeStatus{LifeStatus::Alive};
};

// Reserved for future bullets (mapped from proto when present).
struct BulletPose {
	ObjectID objectId{};
	ObjectID ownerObjectId{};
	Point position{};
	Point velocity{};
};

// Authoritative world frame shared across logic ↔ protocol ↔ app.
struct WorldSnapshot {
	std::uint32_t tick{};
	MapID mapId{};
	std::vector<FigurePose> figures;
	std::vector<BulletPose> bullets;
};

}  // namespace botf::common
