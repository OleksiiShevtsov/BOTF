#pragma once

#include "engine/game_session.h"
#include "engine/game_sessions_pool.h"

#include <cstdint>
#include <optional>

#include <SFML/Network.hpp>

namespace botf::test {

// Bind an ephemeral TCP port and return it (listener is closed afterward).
// Returns nullopt if no free port could be reserved.
[[nodiscard]] inline std::optional<std::uint16_t> ephemeralPort() {
	sf::TcpListener listener;
	listener.setBlocking(true);
	if (listener.listen(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
		return std::nullopt;
	}
	const auto port = static_cast<std::uint16_t>(listener.getLocalPort());
	listener.close();
	if (port == 0) {
		return std::nullopt;
	}
	return port;
}

// Stack-owned test GameSession (card 52). Drive sim through Session facade — not bare GameLogic.
struct TestSession {
	explicit TestSession(const common::ID id = 1) : session{id} {}

	GameSession session;
};

// Pool + one default session for membership tests.
struct TestSessionPool {
	common::ID ensureClient(const common::ID clientId) {
		return pool.ensureDefaultFor(clientId);
	}

	[[nodiscard]] GameSession* sessionById(const common::ID sessionId) {
		return pool.findGameSessionBySessionId(sessionId);
	}

	[[nodiscard]] GameSession* sessionByClient(const common::ID clientId) {
		return pool.findGameSessionByClientId(clientId);
	}

	GameSessionsPool pool;
};

}  // namespace botf::test
