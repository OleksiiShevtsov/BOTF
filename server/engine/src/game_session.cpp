#include "engine/game_session.h"

#include "common/log.hpp"

#include <random>
#include <stdexcept>

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

std::shared_ptr<const botf::GameMap> botf::GameSession::getGameMapPtr() const {
	return m_gameMap;
}

botf::common::Point botf::GameSession::pickRandomSpawnPoint() const {
	if (m_gameMap->getSpawns().empty()) {
		throw std::runtime_error("GameSession: map has no spawn points");
	}
	thread_local std::mt19937 rng{std::random_device{}()};
	std::uniform_int_distribution<std::size_t> dist{0, m_gameMap->getSpawns().size() - 1};
	return m_gameMap->getSpawns()[dist(rng)];
}

void botf::GameSession::seatPlayersRandom() {
	// TODO(card 47): after Waiting→Playing (≥2 clients), shuffle clients onto
	// map spawns (random permutation). Do not seat by join order.
}

botf::common::ID botf::GameSession::bindOrSpawnClient(const common::ID clientId) {
	if (const auto existing = m_gameLogic.entityForClient(clientId)) {
		return *existing;
	}

	// Temporary until card 47 calls seatPlayersRandom() at match start.
	const common::ID entity = m_gameLogic.spawnPlayer(pickRandomSpawnPoint());
	m_gameLogic.bindClient(clientId, entity);
	botf::log::info("[server] client {} bound to entity {} (sessionId={}, mapId={})", clientId,
					entity, m_id, m_gameMap->getId());
	return entity;
}

botf::common::ID botf::GameSession::spawnPlayer(common::Point at) {
	return m_gameLogic.spawnPlayer(at);
}

void botf::GameSession::bindClient(const common::ID clientId, const common::ID entityId) {
	m_gameLogic.bindClient(clientId, entityId);
}

void botf::GameSession::removeClient(const common::ID clientId) {
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
