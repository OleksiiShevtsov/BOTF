#pragma once

#include <cstdint>

namespace botf::common {

// Single id type: TCP client, entity, map, game session, … 
using ID = std::uint32_t;

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
