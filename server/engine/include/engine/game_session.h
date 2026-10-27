#pragma once

#include "common/dto.hpp"
#include "common/types.hpp"
#include "engine/game_map.h"
#include "logic/game_logic.h"

#include <cstddef>

namespace botf {

// One live match: id + GameMap + authoritative GameLogic behind a facade.
// Client membership lives in GameSessionsPool (card 46).
// Phases Waiting/Playing — card 47.
class GameSession {
public:
	GameSession(const common::ID id, GameMap gameMap);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] const GameMap& getGameMap() const;

	// Spawn at next map spawn point + bind.
	common::ID bindOrSpawnClient(const common::ID clientId);

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
	[[nodiscard]] common::Point nextSpawnPoint();

	common::ID m_id{};
	GameMap m_gameMap;
	GameLogic m_gameLogic;
	std::size_t m_nextSpawnIndex{0};
};

}  // namespace botf
