#include "app/client_app.h"

#include "common/global_settings.hpp"
#include "common/log.hpp"
#include "protocol/protocol.hpp"

#include <cmath>
#include <numbers>
#include <optional>
#include <type_traits>

#include <SFML/Config.hpp>
#include <SFML/Network.hpp>
#include <SFML/Window/Mouse.hpp>

namespace {

[[nodiscard]] std::optional<botf::common::Point> selfPosition(
	const std::optional<botf::common::WorldSnapshot>& world, botf::common::ID selfId) {
	if (!world) {
		return std::nullopt;
	}
	for (const auto& pose : world->figures) {
		if (pose.id == selfId) {
			return pose.position;
		}
	}
	return std::nullopt;
}

[[nodiscard]] float angleFromTo(botf::common::Point from, botf::common::Point to) {
	return std::atan2(to.y - from.y, to.x - from.x);
}

[[nodiscard]] float absAngleDelta(float a, float b) {
	float d = a - b;
	constexpr float kPi = std::numbers::pi_v<float>;
	constexpr float kTwoPi = 2.f * kPi;
	while (d > kPi) {
		d -= kTwoPi;
	}
	while (d < -kPi) {
		d += kTwoPi;
	}
	return std::fabs(d);
}

[[nodiscard]] bool aimDue(const botf::common::Point& mouse,
						  const botf::common::Point& lastAim,
						  const std::optional<botf::common::Point>& selfPos) {
	if (selfPos) {
		const float now = angleFromTo(*selfPos, mouse);
		const float prev = angleFromTo(*selfPos, lastAim);
		return absAngleDelta(now, prev) >= botf::common::GlobalSettings::aimSendAngleEpsilon;
	}
	const float dx = mouse.x - lastAim.x;
	const float dy = mouse.y - lastAim.y;
	return dx * dx + dy * dy > botf::common::GlobalSettings::aimSendPointDelta *
								   botf::common::GlobalSettings::aimSendPointDelta;
}

}  // namespace

botf::app::ClientApp::ClientApp() : m_net(m_inbound, m_outbound) {}

void botf::app::ClientApp::sendClientConnect() {
	if (!m_net.isConnected() || m_clientConnectSent) {
		return;
	}

	m_outbound.push(net::OutboundNotification{
		std::nullopt,
		protocol::encodeClientConnect(common::ClientConnectMessage{}, m_sequence),
	});
	++m_sequence;
	m_clientConnectSent = true;
	botf::log::info("[client] ClientConnect sent");
}

void botf::app::ClientApp::applyInbound() {
	while (auto note = m_inbound.tryPop()) {
		auto decoded = protocol::decode(note->payload);
		if (!decoded) {
			botf::log::error("[client] bad envelope from server");
			continue;
		}

		protocol::dispatch(*decoded, [this](const auto& body, std::uint64_t) {
			using Body = std::decay_t<decltype(body)>;
			if constexpr (std::is_same_v<Body, common::ClientConnectMessage>) {
				if (body.boundId == 0) {
					botf::log::warn("[client] ClientConnect ack without object_id");
					return;
				}
				m_selfId = body.boundId;
				m_rendering.getPlayer().setId(*m_selfId);
				botf::log::info("[client] bound to entity {}", *m_selfId);
			} else if constexpr (std::is_same_v<Body, common::WorldSnapshot>) {
				m_latestWorld = body;
			}
			// ClientInput from server is ignored.
		});
	}
}

void botf::app::ClientApp::maybeSendInput(const MoveInput& move, bool fire, std::uint32_t tick) {
	if (!m_net.isConnected() || !m_selfId.has_value()) {
		return;
	}

	const common::Point mouse = m_rendering.mouseWorldPosition();
	if (!m_forceInputSend && !fire && m_lastInputSent) {
		const bool moveDue =
			move.dx != m_lastInputSent->dx || move.dy != m_lastInputSent->dy || (tick % 3u) == 0u;
		const bool aimChanged =
			aimDue(mouse, m_lastInputSent->aim, selfPosition(m_latestWorld, *m_selfId));
		if (!moveDue && !aimChanged) {
			return;
		}
	}

	const common::ClientInput input{move.dx, move.dy, mouse, fire, tick};
	m_outbound.push(net::OutboundNotification{
		std::nullopt,
		protocol::encodeClientInput(input, m_sequence),
	});
	m_lastInputSent = input;
	m_forceInputSend = false;
	++m_sequence;
}

int botf::app::ClientApp::run(std::uint16_t port) {
	botf::log::init();

	m_rendering.getPlayer().setId(0);
	m_rendering.getMatch().start();

	if (!m_net.connect(sf::IpAddress::LocalHost, port, sf::seconds(1))) {
		botf::log::warn("[client] server not available on {} — UI only (input/WorldState offline)",
						port);
	} else {
		botf::log::info("[client] connected to server");
		sendClientConnect();
	}

	std::uint32_t tick = 1;
	while (m_rendering.isWindowOpen()) {
		const MoveInput move = m_rendering.getInputHandler().readMoveInput();
#if SFML_VERSION_MAJOR >= 3
		const bool lmbDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
#else
		const bool lmbDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
#endif
		const bool fireEdge = lmbDown && !m_prevLmbDown;
		m_prevLmbDown = lmbDown;
		maybeSendInput(move, fireEdge, tick);
		++tick;

		if (m_net.isConnected()) {
			m_net.poll();
			if (!m_net.isConnected()) {
				botf::log::warn("[client] disconnected from server");
				m_clientConnectSent = false;
				m_selfId.reset();
				m_forceInputSend = true;
				m_lastInputSent.reset();
				m_prevLmbDown = false;
			}
		}

		applyInbound();

		if (m_latestWorld) {
			m_rendering.getMatch().applyWorldSnapshot(*m_latestWorld, m_selfId);
		}
		m_rendering.getMatch().update();
		m_rendering.draw();
	}

	m_rendering.getMatch().finish();
	m_net.disconnect();
	return 0;
}
