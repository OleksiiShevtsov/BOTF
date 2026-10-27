#include "engine/game_session.h"

#include "common/global_settings.hpp"
#include "common/log.hpp"

botf::GameSession::GameSession(const common::ID id) : m_id{id} {}

botf::common::ID botf::GameSession::getId() const {
	return m_id;
}

botf::common::ID botf::GameSession::bindOrSpawnClient(const common::ID clientId) {
	if (const auto existing = m_gameLogic.entityForClient(clientId)) {
		return *existing;
	}

	const common::ID entity = m_gameLogic.spawnPlayer(
		common::Point{common::GlobalSettings::defaultSpawnX, common::GlobalSettings::defaultSpawnY},
		common::GlobalSettings::defaultPlayerSpeedLimit);
	m_gameLogic.bindClient(clientId, entity);
	botf::log::info("[server] client {} bound to entity {} (sessionId={})", clientId, entity, m_id);
	return entity;
}

botf::common::ID botf::GameSession::spawnPlayer(common::Point at, float speedLimit) {
	return m_gameLogic.spawnPlayer(at, speedLimit);
}

void botf::GameSession::bindClient(const common::ID clientId, const common::ID entityId) {
	m_gameLogic.bindClient(clientId, entityId);
}

void botf::GameSession::removeClient(const common::ID clientId) {
	m_gameLogic.removeClient(clientId);
}

void botf::GameSession::applyClientInput(const common::ID clientId, const common::ClientInput& input) {
	m_gameLogic.setClientInput(clientId, input.dx, input.dy);
	m_gameLogic.setClientAim(clientId, input.aim);
	if (input.fire) {
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
