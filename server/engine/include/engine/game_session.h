#pragma once

#include "common/dto.hpp"
#include "common/types.hpp"
#include "logic/game_logic.h"

namespace botf {

// One live match: id + authoritative GameLogic behind a facade.
// Client membership lives in GameSessionsPool (card 46).
// Map lives on/with GameLogic later (card 51) — not duplicated here.
// Phases Waiting/Playing — card 47.
class GameSession {
public:
	explicit GameSession(const common::ID id);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::ID getId() const;

	// Default spawn + bind (GlobalSettings stub until MapPool / card 51).
	common::ID bindOrSpawnClient(const common::ID clientId);

	// Spawn at an explicit point (tests / future map spawns).
	common::ID spawnPlayer(common::Point at, float speedLimit);
	void bindClient(const common::ID clientId, const common::ID entityId);
	void removeClient(const common::ID clientId);

	void applyClientInput(const common::ID clientId, const common::ClientInput& input);
	bool applyDamage(const common::ID entityId, int damage);
	void tick(float dt);

	[[nodiscard]] common::WorldSnapshot buildWorldSnapshot(std::uint32_t tick) const;

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;
	[[nodiscard]] std::optional<common::ID> entityForClient(const common::ID clientId) const;

private:
	common::ID m_id{};
	GameLogic m_gameLogic;
};

}  // namespace botf
