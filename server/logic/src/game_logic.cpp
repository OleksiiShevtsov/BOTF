#include "logic/game_logic.h"

#include "common/global_settings.hpp"

botf::GameLogic::GameLogic() {
	const common::ID id = spawnPlayer(
		common::Point{common::GlobalSettings::defaultSpawnX, common::GlobalSettings::defaultSpawnY},
		common::GlobalSettings::defaultPlayerSpeedLimit);
	(void)id;
}

botf::common::ID botf::GameLogic::spawnPlayer(common::Point at, float speedLimit) {
	const common::ID id = m_nextEntityId++;
	m_objects.emplace_back(id, at, speedLimit);
	return id;
}

void botf::GameLogic::bindClient(ClientId clientId, common::ID entityId) {
	m_clientToEntity[clientId] = entityId;
}

void botf::GameLogic::setClientInput(ClientId clientId, int dx, int dy) {
	const auto it = m_clientToEntity.find(clientId);
	if (it == m_clientToEntity.end()) {
		return;
	}
	setPlayerInput(it->second, dx, dy);
}

void botf::GameLogic::setPlayerInput(common::ID entityId, int dx, int dy) {
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

std::optional<botf::common::ID> botf::GameLogic::entityForClient(ClientId clientId) const {
	const auto it = m_clientToEntity.find(clientId);
	if (it == m_clientToEntity.end()) {
		return std::nullopt;
	}
	return it->second;
}

botf::DynamicObject* botf::GameLogic::findMutable(common::ID entityId) {
	for (auto& obj : m_objects) {
		if (obj.id() == entityId) {
			return &obj;
		}
	}
	return nullptr;
}
