#pragma once

#include <cstdint>

namespace botf::common {

// TCP session id (assigned by TcpServer). Not a game entity.
using ClientID = std::uint32_t;

// Game / view object id (players, bullets, map objects, …).
using ObjectID = std::uint32_t;

using Coordinate = float;
using Angle = float;

struct Point {
	Coordinate x{};
	Coordinate y{};
};

}  // namespace botf::common
