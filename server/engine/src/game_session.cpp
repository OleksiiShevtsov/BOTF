#include "engine/game_session.h"

#include "common/log.hpp"

#include <stdexcept>
#include <utility>

botf::GameSession::GameSession(const common::ID id, std::shared_ptr<const GameMap> gameMap)
	: m_id{id}, m_gameMap{std::move(gameMap)} {
	if (!m_gameMap) {
		throw std::runtime_error("GameSession: gameMap is null");
	}
	m_spawnTaken.assign(m_gameMap->getSpawns().size(), false);
}

botf::common::ID botf::GameSession::getId() const {
	return m_id;
}

const botf::GameMap& botf::GameSession::getGameMap() const {
	return *m_gameMap;
}

std::optional<std::size_t> botf::GameSession::takeFirstFreeSpawnIndex() {
	for (std::size_t i = 0; i < m_spawnTaken.size(); ++i) {
		if (!m_spawnTaken[i]) {
			m_spawnTaken[i] = true;
			return i;
		}
	}
	return std::nullopt;
}

bool botf::GameSession::hasFreeSeats() const {
	return m_gameLogic.boundClientCount() < m_gameMap->getSpawns().size();
}

void botf::GameSession::seatPlayersRandom() {
	// TODO(card 47): after Waiting→Playing (session full / no free seats),
	// shuffle clients onto map spawns (random permutation). Do not seat by join order.
}

std::optional<botf::common::ID> botf::GameSession::bindOrSpawnClient(const common::ID clientId) {
	if (const auto existing = m_gameLogic.entityForClient(clientId)) {
		return *existing;
	}

	if (!hasFreeSeats()) {
		// TODO(card 47): single-session scaffold — later open a new session and seat there.
		botf::log::warn(
			"[server] TODO(card 47): session {} full (spawns={}); refuse client {} (only one "
			"session for now)",
			m_id, m_gameMap->getSpawns().size(), clientId);
		return std::nullopt;
	}

	const auto spawnIndex = takeFirstFreeSpawnIndex();
	if (!spawnIndex) {
		botf::log::warn("[server] session {} has free seat count but no free spawn index", m_id);
		return std::nullopt;
	}

	const common::Point at = m_gameMap->getSpawns()[*spawnIndex];
	const common::ID entity = m_gameLogic.spawnPlayer(at);
	m_gameLogic.bindClient(clientId, entity);
	m_clientSpawnIndex[clientId] = *spawnIndex;
	botf::log::info("[server] client {} bound to entity {} (sessionId={}, mapId={}, spawn={})",
					clientId, entity, m_id, m_gameMap->getId(), *spawnIndex);
	return entity;
}

botf::common::ID botf::GameSession::spawnPlayer(common::Point at) {
	return m_gameLogic.spawnPlayer(at);
}

void botf::GameSession::bindClient(const common::ID clientId, const common::ID entityId) {
	m_gameLogic.bindClient(clientId, entityId);
}

void botf::GameSession::removeClient(const common::ID clientId) {
	if (const auto it = m_clientSpawnIndex.find(clientId); it != m_clientSpawnIndex.end()) {
		const std::size_t spawnIndex = it->second;
		if (spawnIndex < m_spawnTaken.size()) {
			m_spawnTaken[spawnIndex] = false;
		}
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

bool botf::GameSession::applyDamage(const common::ID entityId, int damage) {
	return m_gameLogic.applyDamage(entityId, damage);
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
