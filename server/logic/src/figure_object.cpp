#include "logic/figure_object.h"

#include "common/global_settings.hpp"

#include <algorithm>
#include <cmath>

namespace {

int clampAxis(int value) {
	return std::clamp(value, -1, 1);
}

}  // namespace

botf::FigureObject::FigureObject(common::ObjectID id, common::Point dot, float speedLimit)
	: DynamicObject{id, dot},
	  m_angle{0.0f},
	  m_speedLimit{speedLimit},
	  m_hp{common::GlobalSettings::maxHp},
	  m_lifeStatus{common::LifeStatus::Alive} {
	m_movement.initialX = m_position.x;
	m_movement.initialY = m_position.y;
}

botf::common::Angle botf::FigureObject::getAngle() const {
	return m_angle;
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
		m_prevInputDx = 0;
		m_prevInputDy = 0;
		return true;
	}
	return false;
}

float botf::FigureObject::speedRecalculation(float acceleration, float initialSpeed) const {
	float speed = acceleration * m_movement.currentTime + initialSpeed;
	if (m_speedLimit > 0.0f) {
		speed = std::clamp(speed, -m_speedLimit, m_speedLimit);
	}
	return speed;
}

botf::common::Coordinate botf::FigureObject::coordinateRecalculation(
	float acceleration, float initialSpeed, common::Coordinate initialCoordinate) const {
	return acceleration * m_movement.currentTime * m_movement.currentTime / 2.0f +
		   initialSpeed * m_movement.currentTime + initialCoordinate;
}

void botf::FigureObject::resetEndpoints() {
	const float speedMag =
		std::sqrt(m_movement.speedX * m_movement.speedX + m_movement.speedY * m_movement.speedY);
	if (speedMag > kSpeedEpsilon) {
		m_movement.cosAcceleration = m_movement.speedX / speedMag;
		m_movement.sinAcceleration = m_movement.speedY / speedMag;
	} else {
		if (m_inputDx != 0 || m_inputDy != 0) {
			const float inputMag =
				std::sqrt(static_cast<float>(m_inputDx * m_inputDx + m_inputDy * m_inputDy));
			m_movement.cosAcceleration = static_cast<float>(m_inputDx) / inputMag;
			m_movement.sinAcceleration =
				static_cast<float>(-m_inputDy) / inputMag;	// +dy = up => -y
		} else {
			m_movement.cosAcceleration = 0.0f;
			m_movement.sinAcceleration = 0.0f;
		}
	}

	m_movement.initialSpeedX = m_movement.speedX;
	m_movement.initialSpeedY = m_movement.speedY;
	m_movement.initialX = m_position.x;
	m_movement.initialY = m_position.y;
	m_movement.currentTime = 0.0f;
}

void botf::FigureObject::update(float dt) {
	if (isDead() || dt <= 0.0f) {
		return;
	}

	if (m_inputDx != m_prevInputDx || m_inputDy != m_prevInputDy) {
		resetEndpoints();
		m_prevInputDx = m_inputDx;
		m_prevInputDy = m_inputDy;
	}

	m_movement.currentTime += dt;

	const bool idle = (m_inputDx == 0 && m_inputDy == 0);

	if (idle) {
		if (std::fabs(m_movement.speedX) > kSpeedEpsilon) {
			m_movement.speedX = speedRecalculation(kFriction * m_movement.cosAcceleration,
												   m_movement.initialSpeedX);
			m_position.x = coordinateRecalculation(kFriction * m_movement.cosAcceleration,
												   m_movement.initialSpeedX, m_movement.initialX);
		}
		if (std::fabs(m_movement.speedY) > kSpeedEpsilon) {
			m_movement.speedY = speedRecalculation(kFriction * m_movement.sinAcceleration,
												   m_movement.initialSpeedY);
			m_position.y = coordinateRecalculation(kFriction * m_movement.sinAcceleration,
												   m_movement.initialSpeedY, m_movement.initialY);
		}
		return;
	}

	if (m_inputDx != 0) {
		const float ax = static_cast<float>(m_inputDx) * kAcceleration;
		m_movement.speedX = speedRecalculation(ax, m_movement.initialSpeedX);
		m_position.x = coordinateRecalculation(ax, m_movement.initialSpeedX, m_movement.initialX);
	} else if (std::fabs(m_movement.speedX) > kSpeedEpsilon) {
		m_movement.speedX =
			speedRecalculation(kFriction * m_movement.cosAcceleration, m_movement.initialSpeedX);
		m_position.x = coordinateRecalculation(kFriction * m_movement.cosAcceleration,
											   m_movement.initialSpeedX, m_movement.initialX);
	}

	if (m_inputDy != 0) {
		const float ay = static_cast<float>(-m_inputDy) * kAcceleration;
		m_movement.speedY = speedRecalculation(ay, m_movement.initialSpeedY);
		m_position.y = coordinateRecalculation(ay, m_movement.initialSpeedY, m_movement.initialY);
	} else if (std::fabs(m_movement.speedY) > kSpeedEpsilon) {
		m_movement.speedY =
			speedRecalculation(kFriction * m_movement.sinAcceleration, m_movement.initialSpeedY);
		m_position.y = coordinateRecalculation(kFriction * m_movement.sinAcceleration,
											   m_movement.initialSpeedY, m_movement.initialY);
	}
}
