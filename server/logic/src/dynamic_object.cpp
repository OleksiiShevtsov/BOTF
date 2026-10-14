#include "logic/dynamic_object.h"

#include <algorithm>
#include <cmath>

namespace {

int clampAxis(int value) {
	return std::clamp(value, -1, 1);
}

}  // namespace

botf::DynamicObject::DynamicObject(common::ID id, common::Point dot, float speedLimit)
	: m_id{id}, m_position{dot}, m_angle{0.0f}, m_speedLimit{speedLimit} {
	m_movement.initialX = m_position.x;
	m_movement.initialY = m_position.y;
}

botf::common::ID botf::DynamicObject::id() const {
	return m_id;
}

botf::common::Point botf::DynamicObject::position() const {
	return m_position;
}

botf::common::Angle botf::DynamicObject::angle() const {
	return m_angle;
}

int botf::DynamicObject::inputDx() const {
	return m_inputDx;
}

int botf::DynamicObject::inputDy() const {
	return m_inputDy;
}

void botf::DynamicObject::setPosition(common::Point dot) {
	m_position = dot;
}

void botf::DynamicObject::setAngle(common::Angle alpha) {
	m_angle = alpha;
}

void botf::DynamicObject::setInput(int dx, int dy) {
	m_inputDx = clampAxis(dx);
	m_inputDy = clampAxis(dy);
}

float botf::DynamicObject::speedRecalculation(float acceleration, float initialSpeed) const {
	float speed = acceleration * m_movement.currentTime + initialSpeed;
	if (m_speedLimit > 0.0f) {
		speed = std::clamp(speed, -m_speedLimit, m_speedLimit);
	}
	return speed;
}

botf::common::Coordinate botf::DynamicObject::coordinateRecalculation(
	float acceleration, float initialSpeed, common::Coordinate initialCoordinate) const {
	return acceleration * m_movement.currentTime * m_movement.currentTime / 2.0f +
		   initialSpeed * m_movement.currentTime + initialCoordinate;
}

void botf::DynamicObject::resetEndpoints() {
	const float speedMag =
		std::sqrt(m_movement.speedX * m_movement.speedX + m_movement.speedY * m_movement.speedY);
	if (speedMag > kSpeedEpsilon) {
		m_movement.cosAcceleration = m_movement.speedX / speedMag;
		m_movement.sinAcceleration = m_movement.speedY / speedMag;
	} else {
		// No velocity yet — friction direction follows current input if any.
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

void botf::DynamicObject::update(float dt) {
	if (dt <= 0.0f) {
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
		// FREE: only friction on axes that still move.
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

	// Active input: accelerate on pressed axes (dy=+1 is up → negative screen/game y).
	// Perpendicular residual speed gets friction (old LEFT/RIGHT/UP/DOWN side-effect).
	if (m_inputDx != 0) {
		const float ax = static_cast<float>(m_inputDx) * kAcceleration;
		m_movement.speedX = speedRecalculation(ax, m_movement.initialSpeedX);
		m_position.x = coordinateRecalculation(ax, m_movement.initialSpeedX, m_movement.initialX);
	} else if (std::fabs(m_movement.speedX) > kSpeedEpsilon) {
		// Fixed UP-branch bug: friction on X uses speedX threshold (not speedY).
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
