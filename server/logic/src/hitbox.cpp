#include "logic/hitbox.h"

#include <cmath>

namespace {

bool pointInAabb(const botf::common::Point& centerHitbox,
				 const float halfW,
				 const float halfH,
				 const botf::common::Point& pointHit) {
	return std::abs(pointHit.x - centerHitbox.x) <= halfW &&
		   std::abs(pointHit.y - centerHitbox.y) <= halfH;
}

// Liang–Barsky clip of segment against axis-aligned box. Inclusive on boundaries.
bool segmentIntersectsAabb(const botf::common::Point& centerHitbox,
						   const float halfW,
						   const float halfH,
						   const botf::common::Point& a,
						   const botf::common::Point& b) {
	const float minX = centerHitbox.x - halfW;
	const float maxX = centerHitbox.x + halfW;
	const float minY = centerHitbox.y - halfH;
	const float maxY = centerHitbox.y + halfH;

	float t0 = 0.f;
	float t1 = 1.f;
	const float dx = b.x - a.x;
	const float dy = b.y - a.y;

	auto clip = [&](const float p, const float q) {
		if (std::abs(p) < 1e-8f) {
			return q >= 0.f;
		}
		const float r = q / p;
		if (p < 0.f) {
			if (r > t1) {
				return false;
			}
			if (r > t0) {
				t0 = r;
			}
		} else {
			if (r < t0) {
				return false;
			}
			if (r < t1) {
				t1 = r;
			}
		}
		return true;
	};

	return clip(-dx, a.x - minX) && clip(dx, maxX - a.x) && clip(-dy, a.y - minY) &&
		   clip(dy, maxY - a.y) && t0 <= t1;
}

}  // namespace

bool botf::pointHitsAabb(const common::Point& centerHitbox,
						 const HitboxDesc& hitbox,
						 const common::Point& pointHit) {
	return pointInAabb(centerHitbox, hitbox.halfWidth(), hitbox.halfHeight(), pointHit);
}

bool botf::segmentHitsAabb(const common::Point& centerHitbox,
						   const HitboxDesc& hitbox,
						   const common::Point& a,
						   const common::Point& b) {
	const float halfW = hitbox.halfWidth();
	const float halfH = hitbox.halfHeight();
	if (a.x == b.x && a.y == b.y) {
		return pointInAabb(centerHitbox, halfW, halfH, a);
	}
	return segmentIntersectsAabb(centerHitbox, halfW, halfH, a, b);
}
