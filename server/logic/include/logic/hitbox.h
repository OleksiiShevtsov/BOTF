#pragma once

#include "common/global_settings.hpp"
#include "common/types.hpp"

namespace botf {

// Axis-aligned hitbox. Entity position + offset = AABB center; does not rotate with aim.
class ObjectHitbox {
public:
	ObjectHitbox() = default;
	ObjectHitbox(float width, float height, float offsetX = 0.f, float offsetY = 0.f)
		: m_width{width}, m_height{height}, m_offsetX{offsetX}, m_offsetY{offsetY} {}

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
	[[nodiscard]] float getOffsetX() const {
		return m_offsetX;
	}
	[[nodiscard]] float getOffsetY() const {
		return m_offsetY;
	}

	// AABB center from entity world position.
	[[nodiscard]] common::Point centerFromEntity(const common::Point& entityPos) const {
		return common::Point{entityPos.x + m_offsetX, entityPos.y + m_offsetY};
	}

	// Default stub — prefer content/figures via FigureDescriptionPool.
	[[nodiscard]] static ObjectHitbox installDefault();

	// Closed AABB: boundary counts as hit. Pass entity position (offset applied inside).
	[[nodiscard]] bool containsPoint(const common::Point& entityPos,
									 const common::Point& pointHit) const;

	// True if segment [a, b] intersects the AABB (inclusive). Degenerate a==b → point test.
	[[nodiscard]] bool intersectsSegment(const common::Point& entityPos,
										 const common::Point& a,
										 const common::Point& b) const;

private:
	float m_width{0.f};
	float m_height{0.f};
	float m_offsetX{0.f};
	float m_offsetY{0.f};
};

}  // namespace botf
