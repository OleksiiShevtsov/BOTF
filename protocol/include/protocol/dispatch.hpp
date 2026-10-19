#pragma once

#include "protocol/messages.hpp"

#include <type_traits>
#include <utility>
#include <variant>

namespace botf::protocol {

// Visit DecodedEnvelope::body, passing (body, sequence) to matching overload.
// Unhandled body types are ignored (no-op). Used by server/client applyInbound.
template <typename Visitor>
void dispatch(const DecodedEnvelope& envelope, Visitor&& visitor) {
	std::visit(
		[&](const auto& body) {
			using Body = std::decay_t<decltype(body)>;
			if constexpr (std::is_invocable_v<Visitor&, const Body&, std::uint64_t>) {
				visitor(body, envelope.sequence);
			}
		},
		envelope.body);
}

}  // namespace botf::protocol
