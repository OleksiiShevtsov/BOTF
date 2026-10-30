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

// ClientConnect / ack (login to server). boundId == 0 on client request.
// Not a GameSession join — that is JoinSession (45/47).
struct ClientConnectMessage {
	std::string playerName;
	ID boundId{};
};

// Client → server: join / find GameSession (card 45). Empty for now; server → 47.
struct JoinSessionMessage {};

// Server → client: match may begin (client Waiting → Playing).
// boundId = entity for this recipient after random seating.
struct SessionStartMessage {
	ID boundId{};
};

// Pose of a controllable actor in a world snapshot.
struct FigurePose {
	ID id{};
	Point position{};
	Angle aimAngle{};
	int hitPoints{};
	LifeStatus lifeStatus{LifeStatus::Alive};
};

// Reserved for future bullets (mapped from proto when present).
struct BulletPose {
	ID id{};
	ID ownerId{};
	Point position{};
	Point velocity{};
};

// Authoritative world frame shared across logic ↔ protocol ↔ app.
struct WorldSnapshot {
	std::uint32_t tick{};
	std::vector<FigurePose> figures;
	std::vector<BulletPose> bullets;
};

}  // namespace botf::common
