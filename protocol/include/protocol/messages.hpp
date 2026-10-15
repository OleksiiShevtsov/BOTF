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

// Extensible: add Input here without touching net framing.
using DecodedMessage = std::variant<DecodedMove, DecodedWorldState, DecodedHello>;

}  // namespace botf::protocol
