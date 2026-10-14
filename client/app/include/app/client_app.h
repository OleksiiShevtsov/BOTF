#pragma once

#include "app/app_rendering.h"
#include "common/dto.hpp"
#include "net/tcp_client.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace botf::app {

[[nodiscard]] std::vector<SceneObject> sceneFromSnapshot(const common::WorldSnapshot& snapshot);

// Client runtime: window + input + TcpClient + WorldSnapshot → scene.
class ClientApp {
public:
	ClientApp();

	ClientApp(const ClientApp&) = delete;
	ClientApp& operator=(const ClientApp&) = delete;

	// Open window and run until closed. Tries localhost:port.
	int run(std::uint16_t port = 54545);

private:
	void applyInbound();
	void maybeSendMove(const MoveInput& input, std::uint32_t tick);

	net::ClientInboundQueue m_inbound;
	net::OutboundQueue m_outbound;
	net::TcpClient m_net;
	AppRendering m_rendering;

	std::uint64_t m_sequence = 1;
	MoveInput m_lastSent{};
	bool m_forceSend = true;
	std::optional<common::WorldSnapshot> m_latestWorld;
};

}  // namespace botf::app
