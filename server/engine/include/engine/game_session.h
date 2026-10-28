#pragma once

#include "common/dto.hpp"
#include "common/types.hpp"
#include "logic/game_map.h"
#include "logic/game_logic.h"

#include <memory>

namespace botf {

// One live match: id + shared GameMap template + authoritative GameLogic facade.
// Client membership lives in GameSessionsPool (card 46).
// Phases Waiting/Playing — card 47.
class GameSession {
public:
	GameSession(const common::ID id, std::shared_ptr<const GameMap> gameMap);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] const GameMap& getGameMap() const;
	[[nodiscard]] std::shared_ptr<const GameMap> getGameMapPtr() const;

	// Scaffold: spawn + bind on connect (temporary until card 47).
	common::ID bindOrSpawnClient(const common::ID clientId);

	// Free seats = map spawns.size() − bound clients. Capacity is spawns only.
	// TODO(card 47): use when joining — if no free seats, do not accept more
	// clients and start Playing instead.
	[[nodiscard]] bool hasFreeSeats() const;

	// TODO(card 47): when session enters Playing (no free seats / enough clients),
	// assign clients to map spawns via random permutation — not join order.
	void seatPlayersRandom();

	// Spawn at an explicit point (tests).
	// Speed limit lives on FigureObject (via GameLogic / GlobalSettings stub).
	common::ID spawnPlayer(common::Point at);
	void bindClient(const common::ID clientId, const common::ID entityId);
	void removeClient(const common::ID clientId);

	void applyClientInput(const common::ID clientId, const common::ClientInput& clientInput);
	bool applyDamage(const common::ID entityId, int damage);
	void tick(float dt);

	[[nodiscard]] common::WorldSnapshot buildWorldSnapshot(std::uint32_t tick) const;

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;
	[[nodiscard]] std::optional<common::ID> entityForClient(const common::ID clientId) const;

private:
	// Temporary connect-time spawn until seatPlayersRandom is wired (card 47).
	[[nodiscard]] common::Point pickRandomSpawnPoint() const;

	common::ID m_id{};
	std::shared_ptr<const GameMap> m_gameMap;
	GameLogic m_gameLogic;
};

}  // namespace botf
