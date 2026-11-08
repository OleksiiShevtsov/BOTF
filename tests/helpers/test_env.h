#pragma once

#include "common/dto.hpp"
#include "engine/bootstrap_storage.h"
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

[[nodiscard]] inline std::shared_ptr<BootstrapStorage> loadBootstrapStorage() {
	auto storage = std::make_shared<BootstrapStorage>();
	if (!storage->load()) {
		return nullptr;
	}
	return storage;
}

[[nodiscard]] inline std::shared_ptr<const GameMap> loadSharedGameMap(
	const common::ID mapId = 1) {
	const auto storage = loadBootstrapStorage();
	if (!storage) {
		return nullptr;
	}
	return storage->gameMapPool().getGameMapById(mapId);
}

struct TestSession {
	explicit TestSession(const common::ID sessionId = 1)
		: storage{loadBootstrapStorage()}, session{sessionId} {
		if (!storage) {
			throw std::runtime_error("TestSession: BootstrapStorage load failed");
		}
	}

	[[nodiscard]] bool beginPlaying() {
		return session.beginPlaying(storage);
	}

	std::shared_ptr<BootstrapStorage> storage;
	GameSession session;
};

struct TestSessionPool {
	TestSessionPool() : storage{loadBootstrapStorage()} {
		if (!storage) {
			throw std::runtime_error("TestSessionPool: BootstrapStorage load failed");
		}
	}

	common::ID joinClient(const common::ID clientId, const common::ID figureTypeId = 1) {
		auto player = std::make_shared<Player>(clientId);
		return pool.attachPlayerToSession(std::move(player), figureTypeId);
	}

	[[nodiscard]] bool beginPlaying(const common::ID sessionId) {
		GameSession* gameSession = sessionById(sessionId);
		if (!gameSession) {
			return false;
		}
		return gameSession->beginPlaying(storage);
	}

	[[nodiscard]] GameSession* sessionById(const common::ID sessionId) {
		return pool.findGameSessionBySessionId(sessionId);
	}

	[[nodiscard]] GameSession* sessionByClient(const common::ID clientId) {
		return pool.findGameSessionByClientId(clientId);
	}

	std::shared_ptr<BootstrapStorage> storage;
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
