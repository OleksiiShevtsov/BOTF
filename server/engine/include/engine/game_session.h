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

// One live match: roster + shared GameMap + phase + GameLogic facade.
// Roster (who is in this match) lives here. Pool keeps clientId→sessionId routing only.
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
	[[nodiscard]] const std::vector<common::ID>& clients() const;

	// Forming and clientCount() < capacity.
	[[nodiscard]] bool hasFreeSeats() const;

	// Forming only. Idempotent if already seated. false if full / wrong phase.
	[[nodiscard]] bool addClient(const common::ID clientId);

	// Forming → Playing: random seats over clients(), spawn+bind. false if refused.
	[[nodiscard]] bool beginPlaying();

	void removeClient(const common::ID clientId);

	void applyClientInput(const common::ID clientId, const common::ClientInput& clientInput);
	void tick(float dt);

	[[nodiscard]] common::WorldSnapshot buildWorldSnapshot(std::uint32_t tick) const;

	[[nodiscard]] const std::vector<FigureObject>& figures() const;
	[[nodiscard]] const std::vector<BulletObject>& bullets() const;
	[[nodiscard]] std::optional<common::ID> entityForClient(const common::ID clientId) const;

private:
	[[nodiscard]] bool containsClient(const common::ID clientId) const;

	common::ID m_id{};
	std::shared_ptr<const GameMap> m_gameMap;
	SessionPhase m_phase{SessionPhase::Forming};
	std::vector<common::ID> m_clients;
	GameLogic m_gameLogic;
	// Spawn index per client — filled only in beginPlaying.
	std::unordered_map<common::ID, std::size_t> m_clientSpawnIndex;
};

}  // namespace botf
