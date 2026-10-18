#pragma once

#include "common/dto.hpp"

#include <cstdint>
#include <variant>

namespace botf::protocol {

// Wire body after Envelope parse. Add new DTO types here when adding messages.
// (ClientInput / bullets stay as proto+DTO stubs until wired.)
using MessageBody = std::
	variant<common::MoveCommand, common::WorldSnapshot, common::HelloMessage, common::AimCommand>;

// decode() result: DTO + envelope sequence (no per-message Decoded* wrappers).
struct DecodedEnvelope {
	MessageBody body{};
	std::uint64_t sequence{};
};

}  // namespace botf::protocol
