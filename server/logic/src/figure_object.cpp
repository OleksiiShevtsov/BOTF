#include "logic/figure_object.h"

#include "common/global_settings.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace {

int clampAxis(int value) {
	return std::clamp(value, -1, 1);
}

// Apply friction along one axis; zero-cross → exactly 0 (no reverse from overshoot).
void applyAxisFriction(float& speed, float friction, float dt) {
	if (speed == 0.f) {
		return;
	}
	const float prev = speed;
	const float delta = (speed > 0.f ? -friction : friction) * dt;
	speed += delta;
	if (prev * speed <= 0.f) {
		speed = 0.f;
	}
}

}  // namespace

botf::FigureObject::FigureObject(const common::ID id, common::Point dot, FigureSpawnParams params)
	: DynamicObject{id, dot},
	  m_angle{0.0f},
	  m_speedLimit{params.speedLimit},
	  m_acceleration{params.acceleration},
	  m_friction{params.friction},
	  m_hp{common::GlobalSettings::maxHp},
	  m_lifeStatus{common::LifeStatus::Alive},
	  m_hitbox{std::move(params.hitbox)} {}

botf::common::Angle botf::FigureObject::getAngle() const {
	return m_angle;
}

const botf::ObjectHitbox& botf::FigureObject::getHitbox() const {
	return m_hitbox;
}

int botf::FigureObject::inputDx() const {
	return m_inputDx;
}

int botf::FigureObject::inputDy() const {
	return m_inputDy;
}

int botf::FigureObject::getHp() const {
	return m_hp;
}

botf::common::LifeStatus botf::FigureObject::getLifeStatus() const {
	return m_lifeStatus;
}

bool botf::FigureObject::isDead() const {
	return m_lifeStatus == common::LifeStatus::Dead || m_hp <= 0;
}

botf::common::Point botf::FigureObject::getVelocity() const {
	return common::Point{m_speedX, m_speedY};
}

void botf::FigureObject::setAngle(common::Angle alpha) {
	if (isDead()) {
		return;
	}
	m_angle = alpha;
}

void botf::FigureObject::setInput(int dx, int dy) {
	if (isDead()) {
		return;
	}
	m_inputDx = clampAxis(dx);
	m_inputDy = clampAxis(dy);
}

void botf::FigureObject::setAimWorld(const common::Point aimWorld) {
	if (isDead()) {
		return;
	}
	m_aimWorld = aimWorld;
	m_hasAimWorld = true;
	retargetFromAim();
}

bool botf::FigureObject::tryFire(const float worldTime) {
	if (isDead()) {
		return false;
	}
	if (m_hasLastFire &&
		worldTime - m_lastFireTime < common::GlobalSettings::fireCooldownSeconds) {
		return false;
	}
	m_lastFireTime = worldTime;
	m_hasLastFire = true;
	return true;
}

bool botf::FigureObject::applyDamage(int damage) {
	if (isDead() || damage <= 0) {
		return false;
	}
	m_hp = std::max(0, m_hp - damage);
	if (m_hp == 0) {
		m_lifeStatus = common::LifeStatus::Dead;
		m_inputDx = 0;
		m_inputDy = 0;
		m_speedX = 0.f;
		m_speedY = 0.f;
		m_hasAimWorld = false;
		return true;
	}
	return false;
}

void botf::FigureObject::clampSpeed() {
	if (m_speedLimit <= 0.f) {
		return;
	}
	const float speed = std::sqrt(m_speedX * m_speedX + m_speedY * m_speedY);
	if (speed > m_speedLimit) {
		const float scale = m_speedLimit / speed;
		m_speedX *= scale;
		m_speedY *= scale;
	}
}

void botf::FigureObject::update(float dt) {
	if (isDead() || dt <= 0.0f) {
		return;
	}

	const bool hasInput = (m_inputDx != 0 || m_inputDy != 0);

	if (hasInput) {
		// Normalize so diagonal does not accelerate √2 faster before speed clamp.
		float ix = static_cast<float>(m_inputDx);
		float iy = static_cast<float>(-m_inputDy);	// +dy = up => -game-y
		const float inputMag = std::sqrt(ix * ix + iy * iy);
		ix /= inputMag;
		iy /= inputMag;

		m_speedX += ix * m_acceleration * dt;
		m_speedY += iy * m_acceleration * dt;

		// Released axis still brakes (no reverse via zero-cross).
		if (m_inputDx == 0) {
			applyAxisFriction(m_speedX, m_friction, dt);
		}
		if (m_inputDy == 0) {
			applyAxisFriction(m_speedY, m_friction, dt);
		}
	} else {
		// Full idle: decelerate along velocity, or per-axis friction — same zero-cross idea.
		applyAxisFriction(m_speedX, m_friction, dt);
		applyAxisFriction(m_speedY, m_friction, dt);
	}

	clampSpeed();

	m_position.x += m_speedX * dt;
	m_position.y += m_speedY * dt;

	retargetFromAim();
}

void botf::FigureObject::retargetFromAim() {
	if (!m_hasAimWorld || isDead()) {
		return;
	}
	const float dx = m_aimWorld.x - m_position.x;
	const float dy = m_aimWorld.y - m_position.y;
	if (dx * dx + dy * dy <
		common::GlobalSettings::aimEpsilon * common::GlobalSettings::aimEpsilon) {
		return;
	}
	m_angle = std::atan2(dy, dx);
}
