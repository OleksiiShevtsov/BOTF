#pragma once

#include <cstdint>

namespace botf {

// Network-facing client id (TCP session).
using ClientId = std::uint32_t;

namespace common {

using Coordinate = float;
using ID = std::uint32_t;
using Angle = float;

struct Point {
	Coordinate x{};
	Coordinate y{};
};

}  // namespace common

}  // namespace botf
