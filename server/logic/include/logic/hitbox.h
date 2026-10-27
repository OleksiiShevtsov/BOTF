#pragma once

#include "common/global_settings.hpp"
#include "common/types.hpp"

namespace botf {

// Axis-aligned hitbox size (card 50). Center = entity position; does not rotate with aim.
// Stub values come from GlobalSettings until DB/config / per-character catalog.
struct ObjectHitbox {
	float width{0.f};
	float height{0.f};

	[[nodiscard]] float getHalfWidth() const {
		return width * 0.5f;
	}
	[[nodiscard]] float getHalfHeight() const {
		return height * 0.5f;
	}

	// Default object hitbox — TODO: нет БД, load from DB / character catalog later.
	[[nodiscard]] static ObjectHitbox installDefault();

	// Closed AABB: boundary counts as hit (triangle vertices sit on the square edge).
	[[nodiscard]] bool containsPoint(const common::Point& centerHitbox,
									 const common::Point& pointHit) const;

	// True if segment [a, b] intersects the AABB (inclusive). Degenerate a==b → point test.
	[[nodiscard]] bool intersectsSegment(const common::Point& centerHitbox,
										 const common::Point& a,
										 const common::Point& b) const;
};

}  // namespace botf
