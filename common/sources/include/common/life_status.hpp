#pragma once

#include <cstdint>

namespace botf::common {

// Authoritative life state (cards 32/35). Waiting = Idle / бездействие.
enum class LifeStatus : std::uint8_t {
	Alive = 0,
	Dead = 1,
	Waiting = 2,
};

}  // namespace botf::common
