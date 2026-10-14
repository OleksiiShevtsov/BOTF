#pragma once

#include "common/types.hpp"

namespace botf::app {

// Client-side view model for drawing (not server logic::DynamicObject).
struct SceneObject {
	common::ID id{};
	common::Point position{};
	common::Angle angle{};
};

struct MoveInput {
	int8_t dx{};
	int8_t dy{};
};

}  // namespace botf::app
