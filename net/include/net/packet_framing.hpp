#pragma once

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

namespace botf::net {

// Packet on the wire: [uint32 little-endian payload size][payload bytes].
// Payload is typically a serialized botf.Envelope (protobuf).
inline constexpr std::size_t kLengthPrefixSize = 4;

inline std::vector<std::uint8_t> framePayload(const std::string& payload) {
	std::vector<std::uint8_t> out(kLengthPrefixSize + payload.size());
	const auto size = static_cast<std::uint32_t>(payload.size());
	out[0] = static_cast<std::uint8_t>(size & 0xFFu);
	out[1] = static_cast<std::uint8_t>((size >> 8) & 0xFFu);
	out[2] = static_cast<std::uint8_t>((size >> 16) & 0xFFu);
	out[3] = static_cast<std::uint8_t>((size >> 24) & 0xFFu);
	if (!payload.empty()) {
		std::memcpy(out.data() + kLengthPrefixSize, payload.data(), payload.size());
	}
	return out;
}

// Appends incoming socket bytes into `buffer`. If a full frame is available,
// returns its payload and removes it from `buffer`.
inline std::optional<std::string> tryConsumeFrame(std::vector<std::uint8_t>& buffer) {
	if (buffer.size() < kLengthPrefixSize) {
		return std::nullopt;
	}

	const std::uint32_t size = static_cast<std::uint32_t>(buffer[0]) |
							   (static_cast<std::uint32_t>(buffer[1]) << 8) |
							   (static_cast<std::uint32_t>(buffer[2]) << 16) |
							   (static_cast<std::uint32_t>(buffer[3]) << 24);

	const std::size_t total = kLengthPrefixSize + static_cast<std::size_t>(size);
	if (buffer.size() < total) {
		return std::nullopt;
	}

	std::string payload(reinterpret_cast<const char*>(buffer.data() + kLengthPrefixSize), size);
	buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(total));
	return payload;
}

}  // namespace botf::net
