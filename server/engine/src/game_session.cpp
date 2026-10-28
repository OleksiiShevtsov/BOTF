#include "engine/game_session.h"

#include "common/log.hpp"

#include <stdexcept>
#include <utility>

botf::GameSession::GameSession(const common::ID id, std::shared_ptr<const GameMap> gameMap)
	: m_id{id}, m_gameMap{std::move(gameMap)} {
	if (!m_gameMap) {
		throw std::runtime_error("GameSession: gameMap is null");
	}
}

botf::common::ID botf::GameSession::getId() const {
	return m_id;
}

const botf::GameMap& botf::GameSession::getGameMap() const {
	return *m_gameMap;
}

std::optional<std::size_t> botf::GameSession::placeInFirstAvailableSpawnPoint(
	const common::ID clientId) {
	const std::vector<common::Point>& spawns = m_gameMap->getSpawns();
	for (std::size_t candidate = 0; candidate < spawns.size(); ++candidate) {
		if (m_takenSpawnIndices.contains(candidate)) {
			continue;
		}
		m_clientSpawnIndex.emplace(clientId, candidate);
		m_takenSpawnIndices.insert(candidate);
		return candidate;
	}
	return std::nullopt;
}

std::optional<botf::common::ID> botf::GameSession::bindOrSpawnClient(const common::ID clientId) {
	if (const auto existing = m_gameLogic.entityForClient(clientId)) {
		return *existing;
	}

	const auto spawnIndex = placeInFirstAvailableSpawnPoint(clientId);
	if (!spawnIndex) {
		// TODO(card 47): single-session scaffold — later open a new session and seat there.
		botf::log::warn(
			"[server] TODO(card 47): session {} full (spawns={}); refuse client {} (only one "
			"session for now)",
			m_id, m_gameMap->getSpawns().size(), clientId);
		return std::nullopt;
	}

	const common::Point at = m_gameMap->getSpawns()[*spawnIndex];
	const common::ID entity = m_gameLogic.spawnPlayer(at);
	m_gameLogic.bindClient(clientId, entity);
	botf::log::info("[server] client {} bound to entity {} (sessionId={}, mapId={}, spawn={})",
					clientId, entity, m_id, m_gameMap->getId(), *spawnIndex);
	return entity;
}

void botf::GameSession::removeClient(const common::ID clientId) {
	const std::unordered_map<common::ID, std::size_t>::iterator it =
		m_clientSpawnIndex.find(clientId);
	if (it != m_clientSpawnIndex.end()) {
		m_takenSpawnIndices.erase(it->second);
		m_clientSpawnIndex.erase(it);
	}
	m_gameLogic.removeClient(clientId);
}

void botf::GameSession::applyClientInput(const common::ID clientId,
										const common::ClientInput& clientInput) {
	m_gameLogic.setClientInput(clientId, clientInput.dx, clientInput.dy);
	m_gameLogic.setClientAim(clientId, clientInput.aim);
	if (clientInput.fire) {
		m_gameLogic.tryFire(clientId);
	}
}

void botf::GameSession::tick(float dt) {
	m_gameLogic.tick(dt);
}

botf::common::WorldSnapshot botf::GameSession::buildWorldSnapshot(std::uint32_t tick) const {
	common::WorldSnapshot snapshot;
	snapshot.tick = tick;
	snapshot.figures.reserve(m_gameLogic.figures().size());
	for (const auto& figure : m_gameLogic.figures()) {
		snapshot.figures.push_back(common::FigurePose{
			figure.getId(),
			figure.getPosition(),
			figure.getAngle(),
			figure.getHp(),
			figure.getLifeStatus(),
		});
	}
	snapshot.bullets.reserve(m_gameLogic.bullets().size());
	for (const auto& bullet : m_gameLogic.bullets()) {
		snapshot.bullets.push_back(common::BulletPose{
			bullet.getId(),
			bullet.getOwnerId(),
			bullet.getPosition(),
			bullet.getVelocity(),
		});
	}
	return snapshot;
}

const std::vector<botf::FigureObject>& botf::GameSession::figures() const {
	return m_gameLogic.figures();
}

const std::vector<botf::BulletObject>& botf::GameSession::bullets() const {
	return m_gameLogic.bullets();
}

std::optional<botf::common::ID> botf::GameSession::entityForClient(const common::ID clientId) const {
	return m_gameLogic.entityForClient(clientId);
}
