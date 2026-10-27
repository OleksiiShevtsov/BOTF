#pragma once

#include "common/global_settings.hpp"
#include "common/types.hpp"

namespace botf {

// Axis-aligned hitbox size (card 50). Center = entity position; does not rotate with aim.
// Stub values come from GlobalSettings until DB/config / per-character catalog.
class ObjectHitbox {
public:
	ObjectHitbox() = default;
	ObjectHitbox(float width, float height) : m_width{width}, m_height{height} {}

	[[nodiscard]] float getWidth() const {
		return m_width;
	}
	[[nodiscard]] float getHeight() const {
		return m_height;
	}
	[[nodiscard]] float getHalfWidth() const {
		return m_width * 0.5f;
	}
	[[nodiscard]] float getHalfHeight() const {
		return m_height * 0.5f;
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

private:
	float m_width{0.f};
	float m_height{0.f};
};

}  // namespace botf
