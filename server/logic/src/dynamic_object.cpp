#include "logic/dynamic_object.h"

botf::DynamicObject::DynamicObject(common::ObjectID id, common::Point position)
	: m_id{id}, m_position{position} {}

botf::common::ObjectID botf::DynamicObject::id() const {
	return m_id;
}

botf::common::Point botf::DynamicObject::position() const {
	return m_position;
}

void botf::DynamicObject::setPosition(common::Point dot) {
	m_position = dot;
}

bool botf::DynamicObject::isAlive() const {
	return true;
}
