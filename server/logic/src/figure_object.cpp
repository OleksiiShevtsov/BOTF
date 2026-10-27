#include "logic/figure_object.h"

#include "common/global_settings.hpp"

#include <algorithm>
#include <cmath>

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

botf::FigureObject::FigureObject(const common::ID id,
								 common::Point dot,
								 float speedLimit,
								 ObjectHitbox hitbox)
	: DynamicObject{id, dot},
	  m_angle{0.0f},
	  m_speedLimit{speedLimit},
	  m_hp{common::GlobalSettings::maxHp},
	  m_lifeStatus{common::LifeStatus::Alive},
	  m_hitbox{hitbox} {}

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

	const float accel = common::GlobalSettings::playerAcceleration;
	const float friction = common::GlobalSettings::playerFriction;
	const bool hasInput = (m_inputDx != 0 || m_inputDy != 0);

	if (hasInput) {
		// Normalize so diagonal does not accelerate √2 faster before speed clamp.
		float ix = static_cast<float>(m_inputDx);
		float iy = static_cast<float>(-m_inputDy);	// +dy = up => -game-y
		const float inputMag = std::sqrt(ix * ix + iy * iy);
		ix /= inputMag;
		iy /= inputMag;

		m_speedX += ix * accel * dt;
		m_speedY += iy * accel * dt;

		// Released axis still brakes (no reverse via zero-cross).
		if (m_inputDx == 0) {
			applyAxisFriction(m_speedX, friction, dt);
		}
		if (m_inputDy == 0) {
			applyAxisFriction(m_speedY, friction, dt);
		}
	} else {
		// Full idle: decelerate along velocity, or per-axis friction — same zero-cross idea.
		applyAxisFriction(m_speedX, friction, dt);
		applyAxisFriction(m_speedY, friction, dt);
	}

	clampSpeed();

	m_position.x += m_speedX * dt;
	m_position.y += m_speedY * dt;
}
