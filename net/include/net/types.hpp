#pragma once

#include "common/types.hpp"

#include <optional>
#include <string>

namespace botf::net {

// Client → server (after length-prefix is stripped).
// `payload` is typically a serialized botf.Envelope.
struct InboundNotification {
	botf::ClientId clientId{};
	std::string payload;
};

// Server → client(s).
// `clientId == nullopt` means broadcast to every connected client.
struct OutboundNotification {
	std::optional<botf::ClientId> clientId;
	std::string payload;
};

// Server → this client (TcpClient inbound). No session id needed.
struct ClientInboundNotification {
	std::string payload;
};

}  // namespace botf::net
