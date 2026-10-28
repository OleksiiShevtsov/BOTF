#pragma once

#include "envelope.pb.h"
#include "protocol/mappers.hpp"
#include "protocol/messages.hpp"

#include <algorithm>
#include <optional>
#include <string>

namespace botf::protocol {
namespace detail {

inline int clampAxis(int value) {
	return std::clamp(value, -1, 1);
}

}  // namespace detail

inline common::ClientInput normalizeInput(common::ClientInput input) {
	input.dx = detail::clampAxis(input.dx);
	input.dy = detail::clampAxis(input.dy);
	return input;
}

inline bool isIdle(const common::ClientInput& input) {
	return input.dx == 0 && input.dy == 0;
}

inline std::string toString(const common::ClientInput& input) {
	const common::ClientInput n = normalizeInput(input);
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

inline std::string encodeClientConnect(const common::ClientConnectMessage& connect,
									   std::uint64_t sequence) {
	botf::Envelope envelope;
	envelope.set_sequence(sequence);
	*envelope.mutable_client_connect() = toProto(connect);
	return envelope.SerializeAsString();
}

inline std::string encodeClientInput(const common::ClientInput& input, std::uint64_t sequence) {
	botf::Envelope envelope;
	envelope.set_sequence(sequence);
	*envelope.mutable_client_input() = toProto(normalizeInput(input));
	return envelope.SerializeAsString();
}

inline std::string encodeWorldSnapshot(const common::WorldSnapshot& snapshot,
									   std::uint64_t sequence) {
	botf::Envelope envelope;
	envelope.set_sequence(sequence);
	*envelope.mutable_world_state() = toProto(snapshot);
	return envelope.SerializeAsString();
}

inline std::string encodeJoinSession(const common::JoinSessionMessage& join,
									 std::uint64_t sequence) {
	botf::Envelope envelope;
	envelope.set_sequence(sequence);
	*envelope.mutable_join_session() = toProto(join);
	return envelope.SerializeAsString();
}

inline std::optional<DecodedEnvelope> decode(const std::string& envelopeBytes) {
	botf::Envelope envelope;
	if (!envelope.ParseFromString(envelopeBytes)) {
		return std::nullopt;
	}

	switch (envelope.body_case()) {
		case botf::Envelope::kClientConnect:
			return DecodedEnvelope{fromProto(envelope.client_connect()), envelope.sequence()};
		case botf::Envelope::kClientInput:
			return DecodedEnvelope{normalizeInput(fromProto(envelope.client_input())),
								   envelope.sequence()};
		case botf::Envelope::kWorldState:
			return DecodedEnvelope{fromProto(envelope.world_state()), envelope.sequence()};
		case botf::Envelope::kJoinSession:
			return DecodedEnvelope{fromProto(envelope.join_session()), envelope.sequence()};
		case botf::Envelope::BODY_NOT_SET:
		default:
			return std::nullopt;
	}
}

}  // namespace botf::protocol
