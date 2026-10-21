#include "logic/bullet_object.h"

#include <cmath>

botf::BulletObject::BulletObject(common::ObjectID id,
								 common::ObjectID ownerId,
								 common::Point origin,
								 common::Angle direction,
								 float speed,
								 float maxRange)
	: DynamicObject{id, origin}
	, m_ownerId{ownerId}
	, m_angle{direction}
	, m_velocity{std::cos(direction) * speed, std::sin(direction) * speed}
	, m_maxRange{maxRange} {}

botf::common::Angle botf::BulletObject::getAngle() const {
	return m_angle;
}

botf::common::ObjectID botf::BulletObject::getOwnerId() const {
	return m_ownerId;
}

botf::common::Point botf::BulletObject::getVelocity() const {
	return m_velocity;
}

bool botf::BulletObject::isAlive() const {
	return m_alive;
}

void botf::BulletObject::destroy() {
	m_alive = false;
}

void botf::BulletObject::update(float dt) {
	if (!m_alive || dt <= 0.f) {
		return;
	}
	m_position.x += m_velocity.x * dt;
	m_position.y += m_velocity.y * dt;
	const float step = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y) * dt;
	m_distanceTraveled += step;
	if (m_distanceTraveled >= m_maxRange) {
		m_alive = false;
	}
}
