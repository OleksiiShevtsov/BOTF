#pragma once

#include "common/types.hpp"
#include "logic/dynamic_object.h"

#include <optional>
#include <unordered_map>
#include <vector>

namespace botf {

// Server-side world simulation. No SFML / no networking.
class GameLogic {
public:
	GameLogic();

	GameLogic(const GameLogic&) = delete;
	GameLogic& operator=(const GameLogic&) = delete;

	// Spawn a controllable entity; returns its logic id.
	common::ID spawnPlayer(common::Point at, float speedLimit);

	// Bind a network client to a logic entity (one entity per client).
	void bindClient(ClientId clientId, common::ID entityId);

	// Apply Move-state input for a network client (no-op if unbound).
	void setClientInput(ClientId clientId, int dx, int dy);

	// Apply input directly by entity id.
	void setPlayerInput(common::ID entityId, int dx, int dy);

	// Advance the world by dt seconds.
	void tick(float dt);

	[[nodiscard]] const std::vector<DynamicObject>& objects() const;
	[[nodiscard]] std::optional<common::ID> entityForClient(ClientId clientId) const;

private:
	DynamicObject* findMutable(common::ID entityId);

	std::vector<DynamicObject> m_objects;
	std::unordered_map<ClientId, common::ID> m_clientToEntity;
	common::ID m_nextEntityId{1};
};

}  // namespace botf
