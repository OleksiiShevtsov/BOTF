#pragma once

#include "common/dto.hpp"
#include "common/types.hpp"
#include "logic/game_logic.h"
#include "logic/game_map.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

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

	// Bind existing client or spawn on first free seat. nullopt if session is full.
	// TODO(card 47): on full session, open another session instead of refusing forever
	// (scaffold still has a single shared session).
	[[nodiscard]] std::optional<common::ID> bindOrSpawnClient(const common::ID clientId);

	// TODO(card 47): when entering Playing, reassign clients to spawns randomly.
	void seatPlayersRandom();

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
	// First map spawn index not held in m_clientSpawnIndex → claim for clientId.
	// TODO(card 47): replace with seatPlayersRandom permutation at Playing.
	[[nodiscard]] std::optional<std::size_t> claimFirstFreeSpawn(const common::ID clientId);

	common::ID m_id{};
	std::shared_ptr<const GameMap> m_gameMap;
	GameLogic m_gameLogic;
	std::unordered_map<common::ID, std::size_t> m_clientSpawnIndex;
};

}  // namespace botf
