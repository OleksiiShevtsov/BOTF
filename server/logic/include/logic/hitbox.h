#pragma once

#include "common/types.hpp"

namespace botf {

// Player hitbox: axis-aligned square centered on entity position (card 34).
// Size from GlobalSettings::hitboxSize; does not rotate with angle.
// Bullet is a point on the server; segment test covers one tick of travel.

[[nodiscard]] float hitboxHalfExtent();

// Closed AABB: boundary counts as hit (triangle vertices sit on the square edge).
[[nodiscard]] bool pointHitsAabb(common::Point center, common::Point point);

// True if segment [a, b] intersects the AABB (inclusive). Degenerate a==b → point test.
[[nodiscard]] bool segmentHitsAabb(common::Point center, common::Point a, common::Point b);

}  // namespace botf
