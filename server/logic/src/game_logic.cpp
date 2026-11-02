#include "logic/game_logic.h"

#include "common/global_settings.hpp"
#include "logic/hitbox.h"

#include <algorithm>
#include <cmath>

botf::GameLogic::GameLogic() = default;

bool botf::GameLogic::spawnFigure(const common::ID entityId, const common::Point at) {
	if (findFigureMutable(entityId) != nullptr) {
		return false;
	}
	m_figures.emplace_back(entityId, at, common::GlobalSettings::defaultPlayerSpeedLimit);
	if (entityId >= m_nextBulletId) {
		m_nextBulletId = entityId + 1;
	}
	return true;
}

void botf::GameLogic::destroyFigure(const common::ID entityId) {
	m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(),
								   [entityId](const BulletObject& b) {
									   return b.getOwnerId() == entityId;
								   }),
					m_bullets.end());
	m_figures.erase(std::remove_if(m_figures.begin(), m_figures.end(),
								   [entityId](const FigureObject& f) { return f.getId() == entityId; }),
					m_figures.end());
	m_entityAimWorld.erase(entityId);
	m_lastFireTime.erase(entityId);
}

void botf::GameLogic::setPlayerInput(const common::ID entityId, const int dx, const int dy) {
	if (FigureObject* obj = findFigureMutable(entityId)) {
		obj->setInput(dx, dy);
	}
}

void botf::GameLogic::setPlayerAim(const common::ID entityId, const common::Point aimWorld) {
	if (findFigureMutable(entityId) == nullptr) {
		return;
	}
	m_entityAimWorld[entityId] = aimWorld;
	applyAimForEntity(entityId);
}

bool botf::GameLogic::tryFire(const common::ID entityId) {
	FigureObject* figure = findFigureMutable(entityId);
	if (!figure || figure->isDead()) {
		return false;
	}

	const auto lastIt = m_lastFireTime.find(entityId);
	if (lastIt != m_lastFireTime.end() &&
		m_worldTime - lastIt->second < common::GlobalSettings::fireCooldownSeconds) {
		return false;
	}

	m_bullets.emplace_back(allocateBulletId(), figure->getId(), figure->getPosition(),
						   figure->getAngle(), common::GlobalSettings::bulletSpeed,
						   common::GlobalSettings::bulletMaxRange);
	m_lastFireTime[entityId] = m_worldTime;
	return true;
}

bool botf::GameLogic::applyDamage(const common::ID entityId, const int damage) {
	FigureObject* figure = findFigureMutable(entityId);
	if (!figure) {
		return false;
	}
	figure->applyDamage(damage);
	return true;
}

void botf::GameLogic::tick(const float dt) {
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

botf::FigureObject* botf::GameLogic::findFigureMutable(const common::ID entityId) {
	for (auto& obj : m_figures) {
		if (obj.getId() == entityId) {
			return &obj;
		}
	}
	return nullptr;
}

void botf::GameLogic::applyAimForEntity(const common::ID entityId) {
	const auto aimIt = m_entityAimWorld.find(entityId);
	if (aimIt == m_entityAimWorld.end()) {
		return;
	}
	FigureObject* obj = findFigureMutable(entityId);
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
	for (const auto& [entityId, _] : m_entityAimWorld) {
		applyAimForEntity(entityId);
	}
}

void botf::GameLogic::resolveBulletHits() {
	const int damage = common::GlobalSettings::bulletDamage;
	for (auto& bullet : m_bullets) {
		if (!bullet.isAlive()) {
			continue;
		}
		const common::Point pointHit = bullet.getPosition();
		const common::ID ownerId = bullet.getOwnerId();

		for (auto& figure : m_figures) {
			if (figure.getId() == ownerId || figure.isDead()) {
				continue;
			}
			if (!figure.getHitbox().containsPoint(figure.getPosition(), pointHit)) {
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

botf::common::ID botf::GameLogic::allocateBulletId() {
	return m_nextBulletId++;
}
