#pragma once

#include "app/types.h"

namespace botf::app {

// Keyboard → MoveInput {dx,dy}. No background thread — call from the client frame loop.
class InputHandler {
public:
	// Axes in {-1, 0, 1}; diagonals allowed. Idle = {0, 0}.
	[[nodiscard]] MoveInput readMoveInput() const;
};

}  // namespace botf::app
