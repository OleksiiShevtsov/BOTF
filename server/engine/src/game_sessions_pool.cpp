#include "engine/game_sessions_pool.h"

#include "common/log.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

botf::common::ID botf::GameSessionsPool::attachPlayerToSession(std::shared_ptr<Player> player,
															   const common::ID figureTypeId) {
	if (!player) {
		throw std::runtime_error("GameSessionsPool::attachPlayerToSession: player is null");
	}
	if (GameSession* existing = findGameSessionByClientId(player->getId())) {
		(void)existing->addPlayer(player, figureTypeId);
		return existing->getId();
	}

	const common::ID clientId = player->getId();
	for (auto& gameSession : m_sessions) {
		if (!gameSession->hasFreeSeats()) {
			continue;
		}
		if (!gameSession->addPlayer(player, figureTypeId)) {
			continue;
		}
		botf::log::info("[server] client {} joined Forming session {} ({}/{}) figureType={}",
						clientId, gameSession->getId(), gameSession->clientCount(),
						gameSession->seatCapacity(), figureTypeId);
		return gameSession->getId();
	}

	GameSession& gameSession = createSession();
	if (!gameSession.addPlayer(std::move(player), figureTypeId)) {
		throw std::runtime_error(
			"GameSessionsPool::attachPlayerToSession: new session rejected player");
	}
	botf::log::info("[server] client {} created Forming session {} (1/{}) figureType={}", clientId,
					gameSession.getId(), gameSession.seatCapacity(), figureTypeId);
	return gameSession.getId();
}

botf::GameSession* botf::GameSessionsPool::findGameSessionByClientId(const common::ID clientId) {
	for (auto& gameSession : m_sessions) {
		if (gameSession->hasClient(clientId)) {
			return gameSession.get();
		}
	}
	return nullptr;
}

const botf::GameSession* botf::GameSessionsPool::findGameSessionByClientId(
	const common::ID clientId) const {
	for (const auto& gameSession : m_sessions) {
		if (gameSession->hasClient(clientId)) {
			return gameSession.get();
		}
	}
	return nullptr;
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
	if (GameSession* gameSession = findGameSessionByClientId(clientId)) {
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
	const common::ID id = m_nextId;
	++m_nextId;
	m_sessions.push_back(std::make_unique<GameSession>(id));
	return *m_sessions.back();
}
