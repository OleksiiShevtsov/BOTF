#pragma once

#include "engine/player.h"

#include <cstddef>
#include <memory>

namespace botf {

// One match slot. Index = figure entity id (1..capacity; 0 reserved on wire).
// Spawn point = gameMap.spawns[index - 1]. After Playing, index is the figure id.
// Holds a non-owning share of ServerEngine's Player (cleared on disconnect).
// Seat is the only bridge between client and the world figure.
class Seat {
public:
	explicit Seat(std::size_t index);

	Seat(const Seat&) = delete;
	Seat& operator=(const Seat&) = delete;

	Seat(Seat&&) noexcept = default;
	Seat& operator=(Seat&&) noexcept = default;

	[[nodiscard]] std::size_t getIndex() const;
	void setIndex(std::size_t index);

	[[nodiscard]] bool isFree() const;

	[[nodiscard]] std::shared_ptr<Player> getPlayer() const;
	void setPlayer(std::shared_ptr<Player> player);
	void clearPlayer();

private:
	std::size_t m_index{};
	std::shared_ptr<Player> m_player{};
};

}  // namespace botf
