#pragma once

#include "common/global_settings.hpp"
#include "common/types.hpp"

namespace botf {

// Axis-aligned hitbox size (card 50). Center = entity position; does not rotate with aim.
// Stub values come from GlobalSettings until DB/config / per-character catalog.
struct HitboxDesc {
	float width{0.f};
	float height{0.f};

	[[nodiscard]] float halfWidth() const {
		return width * 0.5f;
	}
	[[nodiscard]] float halfHeight() const {
		return height * 0.5f;
	}
};

// Default player hitbox — TODO: нет БД, load from DB / character catalog later.
[[nodiscard]] inline HitboxDesc defaultPlayerHitbox() {
	return HitboxDesc{common::GlobalSettings::hitboxSize, common::GlobalSettings::hitboxSize};
}

// Closed AABB: boundary counts as hit (triangle vertices sit on the square edge).
[[nodiscard]] bool pointHitsAabb(const common::Point& centerHitbox,
								 const HitboxDesc& hitbox,
								 const common::Point& pointHit);

// True if segment [a, b] intersects the AABB (inclusive). Degenerate a==b → point test.
[[nodiscard]] bool segmentHitsAabb(const common::Point& centerHitbox,
								   const HitboxDesc& hitbox,
								   const common::Point& a,
								   const common::Point& b);

}  // namespace botf
