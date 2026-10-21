#pragma once

#include <cstdint>

namespace botf::common {

// TCP session id (assigned by TcpServer). Not a game entity.
using ClientID = std::uint32_t;

// Game / view object id (players, bullets, figures, …).
using ObjectID = std::uint32_t;

// Client arena / map selection id (GameMap). Not an ObjectID.
using MapID = std::uint32_t;

using Coordinate = float;
using Angle = float;

struct Point {
	Coordinate x{};
	Coordinate y{};
};

// Authoritative life state (cards 32/35). Waiting = Idle / бездействие.
enum class LifeStatus : std::uint8_t {
	Alive = 0,
	Dead = 1,
	Waiting = 2,
};

}  // namespace botf::common

