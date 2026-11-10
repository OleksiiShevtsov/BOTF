#pragma once

#include "logic/hitbox.h"

namespace botf {

// Server-side figure tuning used at spawn. Filled from content/figures/<id>/figure.json
// for matches; testDefault() mirrors figure id 1 for GameLogic unit tests.
struct FigureSpawnParams {
	ObjectHitbox hitbox{};
	float acceleration{0.f};  // world units / s^2
	float friction{0.f};	  // decelerate when no input on axis / idle
	float speedLimit{0.f};	  // |v| clamp

	[[nodiscard]] static constexpr FigureSpawnParams testDefault() {
		return FigureSpawnParams{ObjectHitbox{40.f, 40.f}, 600.f, 400.f, 200.f};
	}
};

}  // namespace botf
