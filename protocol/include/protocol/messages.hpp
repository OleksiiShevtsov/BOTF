#pragma once

#include "common/dto.hpp"

#include <cstdint>
#include <variant>

namespace botf::protocol {

// Wire body after Envelope oneof parse. Add DTO types here when adding messages.
using MessageBody =
	std::variant<common::ClientConnectMessage, common::ClientInput, common::WorldSnapshot>;

// decode() result: DTO + envelope sequence.
struct DecodedEnvelope {
	MessageBody body{};
	std::uint64_t sequence{};
};

}  // namespace botf::protocol
