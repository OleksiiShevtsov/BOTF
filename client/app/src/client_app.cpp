#include "app/client_app.h"

#include "common/log.hpp"
#include "protocol/protocol.hpp"

#include <variant>

#include <SFML/Network.hpp>

botf::app::ClientApp::ClientApp() : m_net(m_inbound, m_outbound) {}

void botf::app::ClientApp::applyInbound() {
	while (auto note = m_inbound.tryPop()) {
		auto decoded = protocol::decode(note->payload);
		if (!decoded) {
			botf::log::error("[client] bad envelope from server");
			continue;
		}

		const auto* world = std::get_if<protocol::DecodedWorldState>(&*decoded);
		if (!world) {
			continue;
		}
		m_latestWorld = world->snapshot;
	}
}

void botf::app::ClientApp::maybeSendMove(const MoveInput& input, std::uint32_t tick) {
	if (!m_net.isConnected()) {
		return;
	}
	if (!(m_forceSend || input.dx != m_lastSent.dx || input.dy != m_lastSent.dy ||
		  (tick % 3u) == 0u)) {
		return;
	}

	const common::MoveCommand command{input.dx, input.dy, tick};
	m_outbound.push(net::OutboundNotification{
		std::nullopt,
		protocol::encodeMove(command, m_sequence),
	});
	m_lastSent = input;
	m_forceSend = false;
	++m_sequence;
}

int botf::app::ClientApp::run(std::uint16_t port) {
	botf::log::init();

	m_rendering.getPlayer().setId(0);
	m_rendering.getMatch().start();

	if (!m_net.connect(sf::IpAddress::LocalHost, port, sf::seconds(1))) {
		botf::log::warn("[client] server not available on {} — UI only (Move/WorldState offline)",
						port);
	} else {
		botf::log::info("[client] connected to server");
	}

	std::uint32_t tick = 1;
	while (m_rendering.isWindowOpen()) {
		const MoveInput input = m_rendering.getInputHandler().readMoveInput();
		maybeSendMove(input, tick);
		++tick;

		if (m_net.isConnected()) {
			m_net.poll();
			if (!m_net.isConnected()) {
				botf::log::warn("[client] disconnected from server");
			}
		}

		applyInbound();

		if (m_latestWorld) {
			m_rendering.getMatch().applyWorldSnapshot(*m_latestWorld);
		}
		m_rendering.getMatch().update();
		m_rendering.draw();
	}

	m_rendering.getMatch().finish();
	m_net.disconnect();
	return 0;
}
