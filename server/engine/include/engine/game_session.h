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

// Live match phase (card 47). Client UI uses ClientState::Waiting while Forming.
enum class SessionPhase {
	Forming,
	Playing,
	Finished,
};

// One live match: id + shared GameMap + phase + GameLogic facade.
// Client→session membership lives only in GameSessionsPool (no duplicate roster here).
class GameSession {
public:
	GameSession(const common::ID id, std::shared_ptr<const GameMap> gameMap);

	GameSession(const GameSession&) = delete;
	GameSession& operator=(const GameSession&) = delete;

	[[nodiscard]] common::ID getId() const;
	[[nodiscard]] const GameMap& getGameMap() const;
	[[nodiscard]] SessionPhase phase() const;
	[[nodiscard]] std::size_t seatCapacity() const;

	// Forming and occupied < capacity. occupied = pool membership count (source of truth).
	[[nodiscard]] bool hasFreeSeats(std::size_t occupied) const;

	// Forming → Playing: random seat assignment, spawn+bind, then phase Playing.
	// Requires clients.size() == seatCapacity(). Returns false if refused.
	[[nodiscard]] bool beginPlaying(const std::vector<common::ID>& clients);

	void removeClient(const common::ID clientId);

	void applyClientInput(const common::ID clientId, const common::ClientInput& clientInput);
	void tick(float dt);

	[[nodiscard]] common::WorldSnapshot buildWorldSnapshot(std::uint32_t tick) const;

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;
	[[nodiscard]] std::optional<common::ID> entityForClient(const common::ID clientId) const;

private:
	common::ID m_id{};
	std::shared_ptr<const GameMap> m_gameMap;
	SessionPhase m_phase{SessionPhase::Forming};
	GameLogic m_gameLogic;
	// Filled only in beginPlaying (Playing). Not used as membership during Forming.
	std::unordered_map<common::ID, std::size_t> m_clientSpawnIndex;
};

}  // namespace botf
