#pragma once

#include "common/dto.hpp"
#include "engine/game_map_pool.h"
#include "engine/game_session.h"
#include "engine/game_sessions_pool.h"
#include "logic/game_logic.h"
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

// Stack-owned GameSession for session/map tests (no test-only GameSession API).
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

// Apply ClientInput shape to GameLogic (tests of logic — not GameSession).
inline void applyClientInput(GameLogic& logic,
							 const common::ID clientId,
							 const common::ClientInput& clientInput) {
	logic.setClientInput(clientId, clientInput.dx, clientInput.dy);
	logic.setClientAim(clientId, clientInput.aim);
	if (clientInput.fire) {
		logic.tryFire(clientId);
	}
}

inline common::ID spawnBound(GameLogic& logic,
							const common::ID clientId,
							const common::Point at = {0.f, 0.f}) {
	const common::ID entityId = logic.spawnPlayer(at);
	logic.bindClient(clientId, entityId);
	return entityId;
}

}  // namespace botf::test
