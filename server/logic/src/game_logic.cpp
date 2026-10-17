#include "logic/game_logic.h"

#include "common/global_settings.hpp"

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
}

void botf::GameLogic::setClientInput(common::ClientID clientId, int dx, int dy) {
	const auto it = m_clientToEntity.find(clientId);
	if (it == m_clientToEntity.end()) {
		return;
	}
	setPlayerInput(it->second, dx, dy);
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
