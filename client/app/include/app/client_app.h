#pragma once

#include "app/app_rendering.h"
#include "common/dto.hpp"
#include "common/types.hpp"
#include "net/tcp_client.hpp"

#include <cstdint>
#include <optional>

namespace botf::app {

// Client runtime: window + input + TcpClient + WorldSnapshot → Match.
class ClientApp {
public:
	ClientApp();

	ClientApp(const ClientApp&) = delete;
	ClientApp& operator=(const ClientApp&) = delete;

	// Open window and run until closed. Tries localhost:port.
	int run(std::uint16_t port = 54545);

private:
	void sendClientConnect();
	void applyInbound();
	void maybeSendInput(const MoveInput& move, bool fire, std::uint32_t tick);

	net::ClientInboundQueue m_inbound;
	net::OutboundQueue m_outbound;
	net::TcpClient m_net;
	AppRendering m_rendering;

	std::uint64_t m_sequence = 1;
	bool m_clientConnectSent = false;
	bool m_forceInputSend = true;
	bool m_prevLmbDown = false;
	std::optional<common::ClientInput> m_lastInputSent;
	std::optional<common::ObjectID> m_selfId;
	std::optional<common::WorldSnapshot> m_latestWorld;
};

}  // namespace botf::app
