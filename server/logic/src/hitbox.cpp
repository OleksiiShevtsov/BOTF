#include "logic/hitbox.h"

#include "common/global_settings.hpp"

#include <cmath>

namespace {

bool pointInAabb(botf::common::Point center, float half, botf::common::Point point) {
	return std::abs(point.x - center.x) <= half && std::abs(point.y - center.y) <= half;
}

// Liang–Barsky clip of segment against axis-aligned box. Inclusive on boundaries.
bool segmentIntersectsAabb(botf::common::Point center,
						   float half,
						   botf::common::Point a,
						   botf::common::Point b) {
	const float minX = center.x - half;
	const float maxX = center.x + half;
	const float minY = center.y - half;
	const float maxY = center.y + half;

	float t0 = 0.f;
	float t1 = 1.f;
	const float dx = b.x - a.x;
	const float dy = b.y - a.y;

	auto clip = [&](float p, float q) {
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

float botf::hitboxHalfExtent() {
	return common::GlobalSettings::hitboxSize * 0.5f;
}

bool botf::pointHitsAabb(common::Point center, common::Point point) {
	return pointInAabb(center, hitboxHalfExtent(), point);
}

bool botf::segmentHitsAabb(common::Point center, common::Point a, common::Point b) {
	const float half = hitboxHalfExtent();
	if (a.x == b.x && a.y == b.y) {
		return pointInAabb(center, half, a);
	}
	return segmentIntersectsAabb(center, half, a, b);
}
