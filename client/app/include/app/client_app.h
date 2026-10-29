#pragma once

#include "app/app_rendering.h"
#include "app/game_state.h"
#include "common/dto.hpp"
#include "common/types.hpp"
#include "net/tcp_client.hpp"

#include <cstdint>
#include <optional>

namespace botf::app {

// Client runtime: Menu → JoinSession → Waiting → (card 47) Playing.
class ClientApp {
public:
	ClientApp();

	ClientApp(const ClientApp&) = delete;
	ClientApp& operator=(const ClientApp&) = delete;

	// Open window and run until closed. Tries localhost:port on "START GAME".
	int run(std::uint16_t port = 54545);

private:
	void sendJoinSession();
	void onStartGamePressed(std::uint16_t port);
	void returnToMenu();
	void pollNetwork();
	void applyInbound();
	void tickPlaying(bool fire, std::uint32_t& tick);
	void maybeSendInput(const MoveInput& move, bool fire, std::uint32_t tick);
	void present();

	net::ClientInboundQueue m_inbound;
	net::OutboundQueue m_outbound;
	net::TcpClient m_net;
	AppRendering m_rendering;
	GameState m_gameState{GameState::Menu};

	std::uint64_t m_sequence = 1;
	bool m_joinSessionSent = false;
	bool m_forceInputSend = true;
	bool m_prevLmbDown = false;
	std::optional<common::ClientInput> m_lastInputSent;
	std::optional<common::ID> m_selfId;
	std::optional<common::WorldSnapshot> m_latestWorld;
};

}  // namespace botf::app
