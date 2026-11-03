#pragma once

#include "common/dto.hpp"
#include "engine/game_map_pool.h"
#include "engine/game_session.h"
#include "engine/game_sessions_pool.h"
#include "engine/player.h"
#include "logic/game_logic.h"
#include "logic/game_map.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>

#include <SFML/Network.hpp>

namespace botf::test {

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

struct TestSession {
	explicit TestSession(const common::ID sessionId = 1, const common::ID mapId = 1)
		: session{sessionId, loadSharedGameMap(mapId)} {}

	GameSession session;
};

struct TestSessionPool {
	TestSessionPool() : pool{mapPool} {
		if (!mapPool.load()) {
			throw std::runtime_error("TestSessionPool: no maps in content/maps");
		}
	}

	common::ID joinClient(const common::ID clientId) {
		auto player = std::make_shared<Player>(clientId);
		return pool.attachPlayerToSession(std::move(player));
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

inline void applyEntityInput(GameLogic& logic,
							 const common::ID entityId,
							 const common::ClientInput& clientInput) {
	logic.setFigureInput(entityId, clientInput.dx, clientInput.dy);
	logic.setFigureAim(entityId, clientInput.aim);
	if (clientInput.fire) {
		(void)logic.tryFire(entityId);
	}
}

// Allocate the lowest free entity id and spawn (logic unit tests; session uses seat.index).
inline common::ID spawnFigure(GameLogic& logic, const common::Point at = {0.f, 0.f}) {
	for (common::ID id = 0;; ++id) {
		if (logic.spawnFigure(id, at)) {
			return id;
		}
	}
}

}  // namespace botf::test
