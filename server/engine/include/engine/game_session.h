#pragma once

#include "common/dto.hpp"
#include "common/types.hpp"
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

// One live match: seats + shared GameMap + state + GameLogic facade.
// Roster = occupied seats (shared_ptr<Player> from ServerEngine).
class GameSession {
public:
	GameSession(const common::ID id, std::shared_ptr<const GameMap> gameMap);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] const GameMap& getGameMap() const;
	[[nodiscard]] SessionState getSessionState() const;
	[[nodiscard]] std::size_t seatCapacity() const;
	[[nodiscard]] std::size_t clientCount() const;
	[[nodiscard]] std::vector<common::ID> clients() const;
	[[nodiscard]] const std::vector<std::shared_ptr<Seat>>& seats() const;

	[[nodiscard]] bool hasFreeSeats() const;

	// Forming only. Idempotent if already seated. false if full / wrong phase.
	[[nodiscard]] bool addPlayer(std::shared_ptr<Player> player);

	// Forming → Playing: shuffle seat indices, spawn figure id = index. false if refused.
	[[nodiscard]] bool beginPlaying();

	void removePlayer(const common::ID clientId);

	void applyClientInput(const common::ID clientId, const common::ClientInput& clientInput);
	void tick(float dt);

	[[nodiscard]] common::WorldSnapshot buildWorldSnapshot(std::uint32_t tick) const;

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;
	// Seat bridge: occupied seat.index (== figure id). nullopt if client not seated.
	[[nodiscard]] std::optional<common::ID> entityForClient(const common::ID clientId) const;

private:
	[[nodiscard]] std::shared_ptr<Seat> findFreeSeat();
	[[nodiscard]] std::shared_ptr<Seat> findSeatByPlayerId(common::ID clientId);
	[[nodiscard]] std::shared_ptr<Seat> findSeatByPlayerId(common::ID clientId) const;
	void randomShuffleIndex();

	common::ID m_id{};
	std::shared_ptr<const GameMap> m_gameMap;
	SessionState m_sessionState{SessionState::Forming};
	std::vector<std::shared_ptr<Seat>> m_seats;
	GameLogic m_gameLogic;
};

}  // namespace botf
