#include "engine/game_sessions_pool.h"

#include "common/log.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

botf::GameSessionsPool::GameSessionsPool(GameMapPool& gameMapPool)
	: m_gameMapPool(gameMapPool) {}

botf::common::ID botf::GameSessionsPool::attachPlayerToSession(std::shared_ptr<Player> player) {
	if (!player) {
		throw std::runtime_error("GameSessionsPool::attachPlayerToSession: player is null");
	}
	if (GameSession* existing = findGameSessionByClientId(player->getId())) {
		return existing->getId();
	}

	const common::ID clientId = player->getId();
	for (auto& gameSession : m_sessions) {
		if (!gameSession->hasFreeSeats()) {
			continue;
		}
		if (!gameSession->addPlayer(player)) {
			continue;
		}
		const common::ID sessionId = gameSession->getId();
		m_clientToSessionId[clientId] = sessionId;
		botf::log::info("[server] client {} joined Forming session {} ({}/{})", clientId, sessionId,
						gameSession->clientCount(), gameSession->seatCapacity());
		return sessionId;
	}

	GameSession& created = createSession();
	if (!created.addPlayer(std::move(player))) {
		throw std::runtime_error(
			"GameSessionsPool::attachPlayerToSession: new session rejected player");
	}
	m_clientToSessionId[clientId] = created.getId();
	botf::log::info("[server] client {} created Forming session {} (1/{}, mapId={})", clientId,
					created.getId(), created.seatCapacity(), created.getGameMap().getId());
	return created.getId();
}

botf::GameSession* botf::GameSessionsPool::findGameSessionByClientId(const common::ID clientId) {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return nullptr;
	}
	return findGameSessionBySessionId(it->second);
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionByClientId(
	const common::ID clientId) const {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return nullptr;
	}
	return findGameSessionBySessionId(it->second);
}

botf::GameSession* botf::GameSessionsPool::findGameSessionBySessionId(const common::ID sessionId) {
	for (auto& gameSession : m_sessions) {
		if (gameSession->getId() == sessionId) {
			return gameSession.get();
		}
	}
	return nullptr;
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionBySessionId(
	const common::ID sessionId) const {
	for (const auto& gameSession : m_sessions) {
		if (gameSession->getId() == sessionId) {
			return gameSession.get();
		}
	}
	return nullptr;
}

void botf::GameSessionsPool::removePlayer(const common::ID clientId) {
	const auto it = m_clientToSessionId.find(clientId);
	if (it == m_clientToSessionId.end()) {
		return;
	}
	const common::ID sessionId = it->second;
	m_clientToSessionId.erase(it);
	if (GameSession* gameSession = findGameSessionBySessionId(sessionId)) {
		gameSession->removePlayer(clientId);
	}
}

std::vector<botf::common::ID> botf::GameSessionsPool::getSessionClients(
	const common::ID sessionId) const {
	const GameSession* gameSession = findGameSessionBySessionId(sessionId);
	if (!gameSession) {
		return {};
	}
	return gameSession->clients();
}

std::size_t botf::GameSessionsPool::size() const {
	return m_sessions.size();
}

bool botf::GameSessionsPool::empty() const {
	return m_sessions.empty();
}

const std::vector<std::unique_ptr<botf::GameSession>>& botf::GameSessionsPool::sessions() const {
	return m_sessions;
}

std::vector<std::unique_ptr<botf::GameSession>>& botf::GameSessionsPool::sessions() {
	return m_sessions;
}

botf::GameSession& botf::GameSessionsPool::createSession() {
	if (m_gameMapPool.empty()) {
		throw std::runtime_error(
			"GameSessionsPool::createSession: GameMapPool empty (ServerEngine must loadObjectData)");
	}
	const std::shared_ptr<const GameMap> gameMap = m_gameMapPool.pickRandomGameMap();

	const common::ID id = m_nextId;
	++m_nextId;
	m_sessions.push_back(std::make_unique<GameSession>(id, gameMap));
	return *m_sessions.back();
}
