#include "logic/game_logic.h"

#include "common/global_settings.hpp"

#include <algorithm>
#include <utility>

botf::GameLogic::GameLogic() = default;

bool botf::GameLogic::spawnFigure(const common::ID entityId,
								  const common::Point at,
								  FigureSpawnParams params) {
	if (findFigureMutable(entityId) != nullptr) {
		return false;
	}
	m_figures.emplace_back(entityId, at, std::move(params));
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
}

void botf::GameLogic::setFigureInput(const common::ID entityId, const int dx, const int dy) {
	if (FigureObject* obj = findFigureMutable(entityId)) {
		obj->setInput(dx, dy);
	}
}

void botf::GameLogic::setFigureAim(const common::ID entityId, const common::Point aimWorld) {
	if (FigureObject* obj = findFigureMutable(entityId)) {
		obj->setAimWorld(aimWorld);
	}
}

bool botf::GameLogic::tryFire(const common::ID entityId) {
	FigureObject* figure = findFigureMutable(entityId);
	if (!figure || !figure->tryFire(m_worldTime)) {
		return false;
	}

	m_bullets.emplace_back(allocateBulletId(), figure->getId(), figure->getPosition(),
						   figure->getAngle(), common::GlobalSettings::bulletSpeed,
						   common::GlobalSettings::bulletMaxRange);
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
