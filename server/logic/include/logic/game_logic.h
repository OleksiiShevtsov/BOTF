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
	common::ObjectID spawnPlayer(common::Point at, float speedLimit);

	// Bind a network client to a logic entity (one entity per client).
	void bindClient(common::ClientID clientId, common::ObjectID entityId);

	// Drop session → entity mapping (entity stays until destroy path exists).
	void unbindClient(common::ClientID clientId);

	// Apply Move-state input for a network client (no-op if unbound).
	void setClientInput(common::ClientID clientId, int dx, int dy);

	// Aim world point → atan2 from entity pos (no-op if unbound / aim≈pos).
	void setClientAim(common::ClientID clientId, common::Point aimWorld);

	// Apply input directly by entity id.
	void setPlayerInput(common::ObjectID entityId, int dx, int dy);

	// Advance the world by dt seconds.
	void tick(float dt);

	[[nodiscard]] const std::vector<DynamicObject>& objects() const;
	[[nodiscard]] std::optional<common::ObjectID> entityForClient(common::ClientID clientId) const;

private:
	DynamicObject* findMutable(common::ObjectID entityId);

	std::vector<DynamicObject> m_objects;
	std::unordered_map<common::ClientID, common::ObjectID> m_clientToEntity;
	common::ObjectID m_nextEntityId{1};
};

}  // namespace botf
