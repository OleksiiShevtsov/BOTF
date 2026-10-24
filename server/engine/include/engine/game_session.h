#pragma once

#include "common/types.hpp"
#include "logic/game_logic.h"

#include <unordered_set>

namespace botf {

// One live match: mapId + authoritative GameLogic + clients in this session.
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

	void addClient(common::ClientID clientId);
	void removeClient(common::ClientID clientId);
	[[nodiscard]] bool contains(common::ClientID clientId) const;
	[[nodiscard]] const std::unordered_set<common::ClientID>& clients() const;

private:
	static constexpr common::MapID kDefaultMapId{1};

	common::MapID m_mapId{kDefaultMapId};
	GameLogic m_gameLogic;
	std::unordered_set<common::ClientID> m_clients;
};

}  // namespace botf
