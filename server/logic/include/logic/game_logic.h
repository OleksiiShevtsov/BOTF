#pragma once

#include "common/types.hpp"
#include "logic/bullet_object.h"
#include "logic/figure_description.h"
#include "logic/figure_object.h"

#include <vector>

namespace botf {

// Server-side world simulation. No SFML / no networking / no clientId.
// Controllable figures are addressed only by entity id (GameSession seat index).
class GameLogic {
public:
	GameLogic();

	GameLogic(const GameLogic&) = delete;
	GameLogic& operator=(const GameLogic&) = delete;

	// Spawn a controllable figure with an explicit id (seat.index). false if id taken.
	// description from FigureDescriptionPool for matches; testDefault() for logic unit tests.
	[[nodiscard]] bool spawnFigure(common::ID entityId,
								   common::Point at,
								   const FigureDescription& description = FigureDescription::testDefault());

	// Destroy figure and its bullets. No-op if missing.
	void destroyFigure(common::ID entityId);

	// Apply input by entity id. No-op if missing or dead.
	void setFigureInput(common::ID entityId, int dx, int dy);

	// Aim world point on the figure (retarget now + each move tick). No-op if missing.
	void setFigureAim(common::ID entityId, common::Point aimWorld);

	// Rising-edge fire; figure enforces cooldown. false if missing / dead / cooling down.
	[[nodiscard]] bool tryFire(common::ID entityId);

	// Server damage API (card 32). Returns false if entity missing.
	[[nodiscard]] bool applyDamage(common::ID entityId, int damage);

	// Advance the world by dt seconds, resolve hits, prune bullets.
	void tick(float dt);

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;

private:
	FigureObject* findFigureMutable(common::ID entityId);
	// Point ∩ AABB vs living non-owner figures; one hit → damage + destroy bullet.
	void resolveBulletHits();
	void pruneDeadBullets();
	[[nodiscard]] common::ID allocateBulletId();

	std::vector<FigureObject> m_figures;
	std::vector<BulletObject> m_bullets;
	float m_worldTime{0.f};
	// Bullet ids stay above any spawned figure id.
	common::ID m_nextBulletId{0};
};

}  // namespace botf
