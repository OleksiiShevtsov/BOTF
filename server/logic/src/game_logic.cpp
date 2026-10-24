#include "logic/game_logic.h"

#include "common/global_settings.hpp"
#include "logic/hitbox.h"

#include <algorithm>
#include <cmath>

botf::GameLogic::GameLogic() = default;

botf::common::ObjectID botf::GameLogic::spawnPlayer(common::Point at, float speedLimit) {
	const common::ObjectID id = m_nextEntityId++;
	m_figures.emplace_back(id, at, speedLimit);
	return id;
}

void botf::GameLogic::bindClient(common::ClientID clientId, common::ObjectID entityId) {
	m_clientToEntity[clientId] = entityId;
}

void botf::GameLogic::removeClient(common::ClientID clientId) {
	const auto it = m_clientToEntity.find(clientId);
	if (it != m_clientToEntity.end()) {
		const common::ObjectID entityId = it->second;
		m_clientToEntity.erase(it);
		destroyFigure(entityId);
	}
	m_clientAimWorld.erase(clientId);
	m_lastFireTime.erase(clientId);
}

void botf::GameLogic::setClientInput(common::ClientID clientId, int dx, int dy) {
	const auto it = m_clientToEntity.find(clientId);
	if (it == m_clientToEntity.end()) {
		return;
	}
	setPlayerInput(it->second, dx, dy);
}

void botf::GameLogic::setClientAim(common::ClientID clientId, common::Point aimWorld) {
	if (!m_clientToEntity.contains(clientId)) {
		return;
	}
	m_clientAimWorld[clientId] = aimWorld;
	applyAimForClient(clientId);
}

bool botf::GameLogic::tryFire(common::ClientID clientId) {
	const auto bindIt = m_clientToEntity.find(clientId);
	if (bindIt == m_clientToEntity.end()) {
		return false;
	}

	const auto lastIt = m_lastFireTime.find(clientId);
	if (lastIt != m_lastFireTime.end() &&
		m_worldTime - lastIt->second < common::GlobalSettings::fireCooldownSeconds) {
		return false;
	}

	FigureObject* figure = findFigureMutable(bindIt->second);
	if (!figure || figure->isDead()) {
		return false;
	}

	const common::ObjectID bulletId = m_nextEntityId++;
	m_bullets.emplace_back(bulletId, figure->getObjectId(), figure->getPosition(), figure->getAngle(),
						   common::GlobalSettings::bulletSpeed,
						   common::GlobalSettings::bulletMaxRange);
	m_lastFireTime[clientId] = m_worldTime;
	return true;
}

void botf::GameLogic::setPlayerInput(common::ObjectID entityId, int dx, int dy) {
	if (FigureObject* obj = findFigureMutable(entityId)) {
		obj->setInput(dx, dy);
	}
}

bool botf::GameLogic::applyDamage(common::ObjectID entityId, int damage) {
	FigureObject* figure = findFigureMutable(entityId);
	if (!figure) {
		return false;
	}
	figure->applyDamage(damage);
	return true;
}

void botf::GameLogic::tick(float dt) {
	m_worldTime += dt;
	for (auto& figure : m_figures) {
		figure.update(dt);
	}
	applyStoredAims();
	for (auto& bullet : m_bullets) {
		bullet.update(dt);
	}
	resolveBulletHits();
	pruneDeadBullets();
}

const std::vector<botf::FigureObject>& botf::GameLogic::figures() const {
	return m_figures;
}

const std::vector<botf::BulletObject>& botf::GameLogic::bullets() const {
	return m_bullets;
}

std::optional<botf::common::ObjectID> botf::GameLogic::entityForClient(
	common::ClientID clientId) const {
	const auto it = m_clientToEntity.find(clientId);
	if (it == m_clientToEntity.end()) {
		return std::nullopt;
	}
	return it->second;
}

botf::FigureObject* botf::GameLogic::findFigureMutable(common::ObjectID entityId) {
	for (auto& obj : m_figures) {
		if (obj.getObjectId() == entityId) {
			return &obj;
		}
	}
	return nullptr;
}

void botf::GameLogic::destroyFigure(common::ObjectID entityId) {
	m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
								   [entityId](const BulletObject& b) {
									   return b.getOwnerId() == entityId;
								   }),
					m_bullets.end());
	m_figures.erase(std::remove_if(m_figures.begin(), m_figures.end(),
								   [entityId](const FigureObject& f) { return f.getObjectId() == entityId; }),
					m_figures.end());

	for (auto it = m_clientToEntity.begin(); it != m_clientToEntity.end();) {
		if (it->second == entityId) {
			it = m_clientToEntity.erase(it);
		} else {
			++it;
		}
	}
}

void botf::GameLogic::applyAimForClient(common::ClientID clientId) {
	const auto aimIt = m_clientAimWorld.find(clientId);
	if (aimIt == m_clientAimWorld.end()) {
		return;
	}
	const auto bindIt = m_clientToEntity.find(clientId);
	if (bindIt == m_clientToEntity.end()) {
		return;
	}
	FigureObject* obj = findFigureMutable(bindIt->second);
	if (!obj || obj->isDead()) {
		return;
	}

	const common::Point pos = obj->getPosition();
	const float dx = aimIt->second.x - pos.x;
	const float dy = aimIt->second.y - pos.y;
	if (dx * dx + dy * dy <
		common::GlobalSettings::aimEpsilon * common::GlobalSettings::aimEpsilon) {
		return;
	}
	obj->setAngle(std::atan2(dy, dx));
}

void botf::GameLogic::applyStoredAims() {
	for (const auto& [clientId, _] : m_clientAimWorld) {
		applyAimForClient(clientId);
	}
}

void botf::GameLogic::resolveBulletHits() {
	const int damage = common::GlobalSettings::bulletDamage;
	for (auto& bullet : m_bullets) {
		if (!bullet.isAlive()) {
			continue;
		}
		const common::Point pointHit = bullet.getPosition();
		const common::ObjectID ownerId = bullet.getOwnerId();

		for (auto& figure : m_figures) {
			if (figure.getObjectId() == ownerId || figure.isDead()) {
				continue;
			}
			if (!pointHitsAabb(figure.getPosition(), figure.getHitbox(), pointHit)) {
				continue;
			}
			figure.applyDamage(damage);
			bullet.destroy();
			break;
		}
	}
}

void botf::GameLogic::pruneDeadBullets() {
	m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
								   [](const BulletObject& b) { return !b.isAlive(); }),
					m_bullets.end());
}
