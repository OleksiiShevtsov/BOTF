#pragma once

#include "common/dto.hpp"

#include <cstdint>
#include <variant>

namespace botf::protocol {

struct DecodedMove {
	common::MoveCommand command{};
	std::uint64_t sequence{};
};

struct DecodedWorldState {
	common::WorldSnapshot snapshot{};
	std::uint64_t sequence{};
};

struct DecodedHello {
	common::HelloMessage hello{};
	std::uint64_t sequence{};
};

struct DecodedAim {
	common::AimCommand command{};
	std::uint64_t sequence{};
};

// Extensible: add Input here without touching net framing.
using DecodedMessage = std::variant<DecodedMove, DecodedWorldState, DecodedHello, DecodedAim>;

}  // namespace botf::protocol
