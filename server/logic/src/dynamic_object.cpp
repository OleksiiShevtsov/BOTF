#include "logic/dynamic_object.h"

botf::DynamicObject::DynamicObject(const common::ID id, common::Point position)
	: m_id{id}, m_position{position} {}

botf::common::ID botf::DynamicObject::getId() const {
	return m_id;
}

botf::common::Point botf::DynamicObject::getPosition() const {
	return m_position;
}

void botf::DynamicObject::setPosition(common::Point dot) {
	m_position = dot;
}

bool botf::DynamicObject::isAlive() const {
	return true;
}
