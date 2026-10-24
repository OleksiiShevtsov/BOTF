#pragma once

#include "common/types.hpp"
#include "logic/game_logic.h"

namespace botf {

// One live match: mapId + authoritative GameLogic.
// Client membership lives in GameSessionsPool (card 46).
// Phases Waiting/Playing — card 47. MapDesc/spawns — card 51.
class GameSession {
public:
	// Picks stub mapId=1 until MapPool (card 51) supplies real ids.
	GameSession();

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::MapID getMapId() const;
	[[nodiscard]] GameLogic& getGameLogic();
	[[nodiscard]] const GameLogic& getGameLogic() const;

private:
	static constexpr common::MapID kDefaultMapId{1};

	common::MapID m_mapId{kDefaultMapId};
	GameLogic m_gameLogic;
};

}  // namespace botf
