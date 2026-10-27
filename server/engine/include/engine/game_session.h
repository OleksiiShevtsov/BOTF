#pragma once

#include "common/types.hpp"
#include "logic/game_logic.h"

namespace botf {

// One live match: id + authoritative GameLogic.
// Client membership lives in GameSessionsPool (card 46).
// Map lives on/with GameLogic later (card 51) — not duplicated here.
// Phases Waiting/Playing — card 47.
class GameSession {
public:
	explicit GameSession(common::ID id);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] GameLogic& getGameLogic();
	[[nodiscard]] const GameLogic& getGameLogic() const;

private:
	common::ID m_id{};
	GameLogic m_gameLogic;
};

}  // namespace botf
