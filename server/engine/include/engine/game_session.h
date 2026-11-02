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

// Live match phase (card 47). Client UI uses ClientState::Waiting while Forming.
enum class SessionPhase {
	Forming,
	Playing,
	Finished,
};

// One live match: seats + shared GameMap + phase + GameLogic facade.
// Roster = occupied seats (shared_ptr<Player> from ServerEngine).
class GameSession {
public:
	GameSession(const common::ID id, std::shared_ptr<const GameMap> gameMap);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] const GameMap& getGameMap() const;
	[[nodiscard]] SessionPhase phase() const;
	[[nodiscard]] std::size_t seatCapacity() const;
	[[nodiscard]] std::size_t clientCount() const;
	[[nodiscard]] std::vector<common::ID> clients() const;
	[[nodiscard]] const std::vector<Seat>& seats() const;

	[[nodiscard]] bool hasFreeSeats() const;

	// Forming only. Idempotent if already seated. false if full / wrong phase.
	[[nodiscard]] bool addPlayer(std::shared_ptr<Player> player);

	// Forming → Playing: shuffle seat indices (spawn slots), spawn+bind. false if refused.
	[[nodiscard]] bool beginPlaying();

	void removePlayer(const common::ID clientId);

	void applyClientInput(const common::ID clientId, const common::ClientInput& clientInput);
	void tick(float dt);

	[[nodiscard]] common::WorldSnapshot buildWorldSnapshot(std::uint32_t tick) const;

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;
	[[nodiscard]] std::optional<common::ID> entityForClient(const common::ID clientId) const;

private:
	[[nodiscard]] Seat* findFreeSeat();
	[[nodiscard]] Seat* findSeatByPlayerId(common::ID clientId);
	[[nodiscard]] const Seat* findSeatByPlayerId(common::ID clientId) const;

	common::ID m_id{};
	std::shared_ptr<const GameMap> m_gameMap;
	SessionPhase m_phase{SessionPhase::Forming};
	std::vector<Seat> m_seats;
	GameLogic m_gameLogic;
};

}  // namespace botf
