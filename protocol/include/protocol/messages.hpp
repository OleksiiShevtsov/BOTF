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

// Extensible: add Hello / Input here without touching net framing.
using DecodedMessage = std::variant<DecodedMove, DecodedWorldState>;

}  // namespace botf::protocol
