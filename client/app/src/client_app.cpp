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

[[nodiscard]] bool isLeftMouseDown() {
#if SFML_VERSION_MAJOR >= 3
	return sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
#else
	return sf::Mouse::isButtonPressed(sf::Mouse::Left);
#endif
}

}  // namespace

botf::app::ClientApp::ClientApp() : m_net(m_inbound, m_outbound) {}

void botf::app::ClientApp::sendJoinSession() {
	if (!m_net.isConnected() || m_joinSessionSent) {
		return;
	}

	m_outbound.push(net::OutboundNotification{
		std::nullopt,
		protocol::encodeJoinSession(common::JoinSessionMessage{}, m_sequence),
	});
	++m_sequence;
	m_joinSessionSent = true;
	botf::log::info("[client] JoinSession sent");
}

void botf::app::ClientApp::onStartGamePressed(const std::uint16_t port) {
	if (m_clientState != common::ClientState::Menu) {
		return;
	}

	if (!m_net.isConnected()) {
		if (!m_net.connect(sf::IpAddress::LocalHost, port, sf::seconds(1))) {
			botf::log::warn("[client] server not available on {} — JoinSession not sent", port);
			return;
		}
		botf::log::info("[client] connected to server");
	}

	// Card 45: do not send ClientConnect (login/spawn). JoinSession only — server ignores
	// until card 47; match path is intentionally broken until then.
	sendJoinSession();
	m_clientState = common::ClientState::Waiting;
}

void botf::app::ClientApp::returnToMenu() {
	m_joinSessionSent = false;
	m_selfId.reset();
	m_forceInputSend = true;
	m_lastInputSent.reset();
	m_latestWorld.reset();
	m_prevLmbDown = false;
	m_rendering.getMatch().finish();
	m_clientState = common::ClientState::Menu;
}

void botf::app::ClientApp::pollNetwork() {
	if (!m_net.isConnected()) {
		return;
	}

	m_net.poll();
	if (!m_net.isConnected()) {
		botf::log::warn("[client] disconnected from server");
		returnToMenu();
		return;
	}
	applyInbound();
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
			// ClientInput / JoinSession from server are ignored.
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

void botf::app::ClientApp::tickPlaying(const bool fire, std::uint32_t& tick) {
	const MoveInput move = m_rendering.getInputHandler().readMoveInput();
	maybeSendInput(move, fire, tick);
	++tick;

	pollNetwork();
	if (m_clientState != common::ClientState::Playing) {
		return;
	}

	if (m_latestWorld) {
		m_rendering.getMatch().applyWorldSnapshot(*m_latestWorld, m_selfId);
	}
	m_rendering.getMatch().update();
}

void botf::app::ClientApp::present() {
	switch (m_clientState) {
	case common::ClientState::Menu:
		m_rendering.drawMenu();
		break;
	case common::ClientState::Waiting:
		m_rendering.drawWaiting();
		break;
	case common::ClientState::Playing:
		m_rendering.drawMatch();
		break;
	}
}

int botf::app::ClientApp::run(std::uint16_t port) {
	botf::log::init();

	m_rendering.getPlayer().setId(0);
	m_clientState = common::ClientState::Menu;
	if (!m_rendering.getMenuScreen().isReady()) {
		botf::log::error("[client] menu font missing (content/ui/DejaVuSans.ttf)");
	}

	std::uint32_t tick = 1;
	while (m_rendering.isWindowOpen()) {
		const bool lmbDown = isLeftMouseDown();
		const bool lmbPressed = lmbDown && !m_prevLmbDown;
		m_prevLmbDown = lmbDown;

		switch (m_clientState) {
		case common::ClientState::Menu: {
			MenuScreen& menu = m_rendering.getMenuScreen();
			const common::Point mouse = m_rendering.mouseWorldPosition();
			menu.setHovered(menu.containsPoint(mouse));
			if (lmbPressed && menu.containsPoint(mouse)) {
				onStartGamePressed(port);
			}
			break;
		}
		case common::ClientState::Waiting:
			// Hold TCP + listen only. No Match / combat input until Playing (card 47).
			pollNetwork();
			break;
		case common::ClientState::Playing:
			tickPlaying(lmbPressed, tick);
			break;
		}

		present();
	}

	m_rendering.getMatch().finish();
	m_net.disconnect();
	return 0;
}
