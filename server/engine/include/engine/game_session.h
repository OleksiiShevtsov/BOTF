#pragma once

#include "common/types.hpp"
#include "logic/game_logic.h"

#include <unordered_set>

namespace botf {

// One live match: mapId + authoritative GameLogic + clients in this session.
// Phases Waiting/Playing — card 47. MapDesc/spawns — card 51.
class GameSession {
public:
	explicit GameSession(common::MapID mapId);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::MapID mapId() const;
	[[nodiscard]] GameLogic& logic();
	[[nodiscard]] const GameLogic& logic() const;

	void addClient(common::ClientID clientId);
	void removeClient(common::ClientID clientId);
	[[nodiscard]] bool contains(common::ClientID clientId) const;
	[[nodiscard]] const std::unordered_set<common::ClientID>& clients() const;

private:
	common::MapID m_mapId{};
	GameLogic m_logic;
	std::unordered_set<common::ClientID> m_clients;
};

}  // namespace botf
