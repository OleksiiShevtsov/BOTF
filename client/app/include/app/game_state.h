#pragma once

namespace botf::app {

// Client UI / session flow (card 45). Playing entered from Waiting → card 47.
enum class GameState {
	Menu,
	Waiting,
	Playing,
};

}  // namespace botf::app
