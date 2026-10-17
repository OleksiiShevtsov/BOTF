#include "logic/game_logic.h"

#include "common/global_settings.hpp"

#include <cmath>

botf::GameLogic::GameLogic() = default;

botf::common::ObjectID botf::GameLogic::spawnPlayer(common::Point at, float speedLimit) {
	const common::ObjectID id = m_nextEntityId++;
	m_objects.emplace_back(id, at, speedLimit);
	return id;
}

void botf::GameLogic::bindClient(common::ClientID clientId, common::ObjectID entityId) {
	m_clientToEntity[clientId] = entityId;
}

void botf::GameLogic::unbindClient(common::ClientID clientId) {
	m_clientToEntity.erase(clientId);
	m_clientAimWorld.erase(clientId);
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

void botf::GameLogic::setPlayerInput(common::ObjectID entityId, int dx, int dy) {
	if (DynamicObject* obj = findMutable(entityId)) {
		obj->setInput(dx, dy);
	}
}

void botf::GameLogic::tick(float dt) {
	for (auto& obj : m_objects) {
		obj.update(dt);
	}
	applyStoredAims();
}

const std::vector<botf::DynamicObject>& botf::GameLogic::objects() const {
	return m_objects;
}

std::optional<botf::common::ObjectID> botf::GameLogic::entityForClient(
	common::ClientID clientId) const {
	const auto it = m_clientToEntity.find(clientId);
	if (it == m_clientToEntity.end()) {
		return std::nullopt;
	}
	return it->second;
}

botf::DynamicObject* botf::GameLogic::findMutable(common::ObjectID entityId) {
	for (auto& obj : m_objects) {
		if (obj.id() == entityId) {
			return &obj;
		}
	}
	return nullptr;
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
	DynamicObject* obj = findMutable(bindIt->second);
	if (!obj) {
		return;
	}

	const common::Point pos = obj->position();
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
