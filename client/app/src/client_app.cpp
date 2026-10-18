#include "app/client_app.h"

#include "common/global_settings.hpp"
#include "common/log.hpp"
#include "protocol/protocol.hpp"

#include <cmath>
#include <numbers>
#include <optional>
#include <variant>

#include <SFML/Network.hpp>

namespace {

[[nodiscard]] std::optional<botf::common::Point> selfPosition(
	const std::optional<botf::common::WorldSnapshot>& world, botf::common::ObjectID selfId) {
	if (!world) {
		return std::nullopt;
	}
	for (const auto& pose : world->players) {
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

}  // namespace

botf::app::ClientApp::ClientApp() : m_net(m_inbound, m_outbound) {}

void botf::app::ClientApp::sendHello() {
	if (!m_net.isConnected() || m_helloSent) {
		return;
	}

	m_outbound.push(net::OutboundNotification{
		std::nullopt,
		protocol::encodeHello(common::HelloMessage{}, m_sequence),
	});
	++m_sequence;
	m_helloSent = true;
	botf::log::info("[client] Hello sent");
}

void botf::app::ClientApp::applyInbound() {
	while (auto note = m_inbound.tryPop()) {
		auto decoded = protocol::decode(note->payload);
		if (!decoded) {
			botf::log::error("[client] bad envelope from server");
			continue;
		}

		if (const auto* hello = std::get_if<common::HelloMessage>(&decoded->body)) {
			if (hello->objectId == 0) {
				botf::log::warn("[client] Hello ack without object_id");
				continue;
			}
			m_selfId = hello->objectId;
			m_rendering.getPlayer().setId(*m_selfId);
			botf::log::info("[client] bound to entity {}", *m_selfId);
			continue;
		}

		if (const auto* world = std::get_if<common::WorldSnapshot>(&decoded->body)) {
			m_latestWorld = *world;
		}
	}
}

void botf::app::ClientApp::maybeSendMove(const MoveInput& input, std::uint32_t tick) {
	if (!m_net.isConnected() || !m_selfId.has_value()) {
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

void botf::app::ClientApp::maybeSendAim(std::uint32_t tick) {
	if (!m_net.isConnected() || !m_selfId.has_value()) {
		return;
	}

	const common::Point mouse = m_rendering.mouseWorldPosition();
	// Every frame when aim meaningfully changes (no tick%3 throttle).
	// Prefer angular delta vs self pose; fall back to a low point delta.
	if (!m_forceAimSend && m_lastAimSent) {
		bool shouldSend = false;
		if (const auto selfPos = selfPosition(m_latestWorld, *m_selfId)) {
			const float now = angleFromTo(*selfPos, mouse);
			const float prev = angleFromTo(*selfPos, *m_lastAimSent);
			shouldSend = absAngleDelta(now, prev) >= common::GlobalSettings::aimSendAngleEpsilon;
		} else {
			const float dx = mouse.x - m_lastAimSent->x;
			const float dy = mouse.y - m_lastAimSent->y;
			const float thr = common::GlobalSettings::aimSendPointDelta;
			shouldSend = dx * dx + dy * dy > thr * thr;
		}
		if (!shouldSend) {
			return;
		}
	}

	m_outbound.push(net::OutboundNotification{
		std::nullopt,
		protocol::encodeAim(common::AimCommand{mouse, tick}, m_sequence),
	});
	m_lastAimSent = mouse;
	m_forceAimSend = false;
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
		sendHello();
	}

	std::uint32_t tick = 1;
	while (m_rendering.isWindowOpen()) {
		const MoveInput input = m_rendering.getInputHandler().readMoveInput();
		maybeSendMove(input, tick);
		maybeSendAim(tick);
		++tick;

		if (m_net.isConnected()) {
			m_net.poll();
			if (!m_net.isConnected()) {
				botf::log::warn("[client] disconnected from server");
				m_helloSent = false;
				m_selfId.reset();
				m_forceAimSend = true;
				m_lastAimSent.reset();
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
