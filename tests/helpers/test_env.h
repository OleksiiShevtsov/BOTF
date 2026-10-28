#pragma once

#include "engine/game_map_pool.h"
#include "engine/game_session.h"
#include "engine/game_sessions_pool.h"
#include "logic/game_map.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>

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

[[nodiscard]] inline std::shared_ptr<const GameMap> loadSharedGameMap(
	const common::ID mapId = 1) {
	GameMapPool mapPool;
	if (!mapPool.load()) {
		return nullptr;
	}
	return mapPool.getGameMapById(mapId);
}

// Stack-owned test GameSession with shared map from GameMapPool (card 51/52).
struct TestSession {
	explicit TestSession(const common::ID sessionId = 1, const common::ID mapId = 1)
		: session{sessionId, loadSharedGameMap(mapId)} {}

	GameSession session;
};

// Map catalog + sessions pool for membership tests (mirrors ServerEngine ownership).
struct TestSessionPool {
	TestSessionPool() : pool{mapPool} {
		if (!mapPool.load()) {
			throw std::runtime_error("TestSessionPool: no maps in content/maps");
		}
	}

	common::ID ensureClient(const common::ID clientId) {
		return pool.ensureDefaultFor(clientId);
	}

	[[nodiscard]] GameSession* sessionById(const common::ID sessionId) {
		return pool.findGameSessionBySessionId(sessionId);
	}

	[[nodiscard]] GameSession* sessionByClient(const common::ID clientId) {
		return pool.findGameSessionByClientId(clientId);
	}

	GameMapPool mapPool;
	GameSessionsPool pool;
};

}  // namespace botf::test
