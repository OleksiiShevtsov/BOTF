#pragma once

#include "common/dto.hpp"
#include "common/types.hpp"
#include "engine/bootstrap_storage.h"
#include "engine/player.h"
#include "engine/seat.h"
#include "logic/game_logic.h"
#include "logic/game_map.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace botf {

// Live match state (card 47). Client UI uses ClientState::Waiting while Forming.
enum class SessionState {
	Forming,
	Playing,
	Finished,
};

// One live match: seats + GameMap (after beginPlaying) + GameLogic facade.
// Roster = occupied seats (shared_ptr<Player> from ServerEngine).
class GameSession {
public:
	explicit GameSession(const common::ID id);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] bool hasGameMap() const;
	[[nodiscard]] const GameMap& getGameMap() const;
	[[nodiscard]] SessionState getSessionState() const;
	[[nodiscard]] std::size_t seatCapacity() const;
	[[nodiscard]] std::size_t clientCount() const;
	[[nodiscard]] std::vector<common::ID> clients() const;
	[[nodiscard]] const std::vector<std::shared_ptr<Seat>>& seats() const;

	[[nodiscard]] bool hasFreeSeats() const;

	// Forming only. Idempotent if already seated. false if full / wrong phase.
	[[nodiscard]] bool addPlayer(std::shared_ptr<Player> player, common::ID figureTypeId);

	// Forming → Playing: pick map + spawn figures from BootstrapStorage.
	[[nodiscard]] bool beginPlaying(std::shared_ptr<BootstrapStorage> storage);

	void removePlayer(const common::ID clientId);

	void applyClientInput(const common::ID clientId, const common::ClientInput& clientInput);
	void tick(float dt);

	[[nodiscard]] common::WorldSnapshot buildWorldSnapshot(std::uint32_t tick) const;

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;
	[[nodiscard]] bool hasClient(const common::ID clientId) const;
	// Occupied seat.index (== figure id). nullopt if client not seated.
	[[nodiscard]] std::optional<common::ID> figureIdForClient(const common::ID clientId) const;

private:
	[[nodiscard]] std::shared_ptr<Seat> findFreeSeat();
	[[nodiscard]] std::shared_ptr<Seat> findSeatByPlayerId(common::ID clientId);
	[[nodiscard]] std::shared_ptr<Seat> findSeatByPlayerId(common::ID clientId) const;

	common::ID m_id{};
	std::shared_ptr<const GameMap> m_gameMap;
	SessionState m_sessionState{SessionState::Forming};
	std::vector<std::shared_ptr<Seat>> m_seats;
	GameLogic m_gameLogic;
};

}  // namespace botf
