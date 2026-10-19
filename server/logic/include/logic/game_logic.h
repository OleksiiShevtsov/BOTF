#pragma once

#include "common/types.hpp"
#include "logic/bullet_object.h"
#include "logic/figure_object.h"

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

	// Spawn a controllable figure; returns its logic id.
	common::ObjectID spawnPlayer(common::Point at, float speedLimit);

	// Bind a network client to a logic entity (one entity per client).
	void bindClient(common::ClientID clientId, common::ObjectID entityId);

	// Drop session → entity mapping (entity stays until destroy path exists).
	void unbindClient(common::ClientID clientId);

	// Apply move input for a network client (no-op if unbound).
	void setClientInput(common::ClientID clientId, int dx, int dy);

	// Store aim world point for the bound entity; angle applied now and each tick.
	void setClientAim(common::ClientID clientId, common::Point aimWorld);

	// Rising-edge fire from ClientInput; enforces cooldown. Returns true if spawned.
	bool tryFire(common::ClientID clientId);

	// Apply input directly by entity id.
	void setPlayerInput(common::ObjectID entityId, int dx, int dy);

	// Advance the world by dt seconds, retarget aims, prune dead bullets.
	void tick(float dt);

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;
	[[nodiscard]] std::optional<common::ObjectID> entityForClient(common::ClientID clientId) const;

private:
	FigureObject* findFigureMutable(common::ObjectID entityId);
	void applyAimForClient(common::ClientID clientId);
	void applyStoredAims();
	void pruneDeadBullets();

	std::vector<FigureObject> m_figures;
	std::vector<BulletObject> m_bullets;
	std::unordered_map<common::ClientID, common::ObjectID> m_clientToEntity;
	// Last aim world point per client; angle = atan2(aim − pos) after each move tick.
	std::unordered_map<common::ClientID, common::Point> m_clientAimWorld;
	std::unordered_map<common::ClientID, float> m_lastFireTime;
	float m_worldTime{0.f};
	common::ObjectID m_nextEntityId{1};
};

}  // namespace botf
