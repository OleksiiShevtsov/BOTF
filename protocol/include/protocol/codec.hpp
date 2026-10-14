#pragma once

#include "envelope.pb.h"
#include "protocol/mappers.hpp"
#include "protocol/messages.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <variant>

namespace botf::protocol {
namespace detail {

inline int clampAxis(int value) {
	return std::clamp(value, -1, 1);
}

}  // namespace detail

inline common::MoveCommand normalizeMove(common::MoveCommand command) {
	command.dx = detail::clampAxis(command.dx);
	command.dy = detail::clampAxis(command.dy);
	return command;
}

inline bool isIdle(const common::MoveCommand& command) {
	return command.dx == 0 && command.dy == 0;
}

inline std::string toString(const common::MoveCommand& command) {
	const common::MoveCommand n = normalizeMove(command);
	if (isIdle(n)) {
		return "IDLE";
	}

	std::string out;
	if (n.dy > 0) {
		out += "UP";
	} else if (n.dy < 0) {
		out += "DOWN";
	}
	if (n.dx > 0) {
		if (!out.empty()) {
			out += "+";
		}
		out += "RIGHT";
	} else if (n.dx < 0) {
		if (!out.empty()) {
			out += "+";
		}
		out += "LEFT";
	}
	return out;
}

inline std::string encodeMove(const common::MoveCommand& command, std::uint64_t sequence) {
	const common::MoveCommand normalized = normalizeMove(command);
	botf::Envelope envelope;
	envelope.set_type(botf::MESSAGE_TYPE_MOVE);
	envelope.set_sequence(sequence);
	envelope.set_payload(toProto(normalized).SerializeAsString());
	return envelope.SerializeAsString();
}

inline std::string encodeWorldSnapshot(const common::WorldSnapshot& snapshot,
									   std::uint64_t sequence) {
	botf::Envelope envelope;
	envelope.set_type(botf::MESSAGE_TYPE_WORLD_STATE);
	envelope.set_sequence(sequence);
	envelope.set_payload(toProto(snapshot).SerializeAsString());
	return envelope.SerializeAsString();
}

inline std::optional<DecodedMessage> decode(const std::string& envelopeBytes) {
	botf::Envelope envelope;
	if (!envelope.ParseFromString(envelopeBytes)) {
		return std::nullopt;
	}

	switch (envelope.type()) {
		case botf::MESSAGE_TYPE_MOVE: {
			botf::Move move;
			if (!move.ParseFromString(envelope.payload())) {
				return std::nullopt;
			}
			return DecodedMessage{DecodedMove{normalizeMove(fromProto(move)), envelope.sequence()}};
		}
		case botf::MESSAGE_TYPE_WORLD_STATE: {
			botf::WorldState state;
			if (!state.ParseFromString(envelope.payload())) {
				return std::nullopt;
			}
			return DecodedMessage{DecodedWorldState{fromProto(state), envelope.sequence()}};
		}
		default:
			return std::nullopt;
	}
}

}  // namespace botf::protocol
