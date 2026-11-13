#pragma once

#include "common/types.hpp"
#include "engine/player.h"

#include <memory>

namespace botf {

// One match slot. Index = figure entity id = spawn slot (0..capacity-1).
// After Playing, index is the figure id and indexes gameMap.spawns[index].
// Holds a non-owning share of ServerEngine's Player (cleared on disconnect).
// Seat is the only bridge between client and the world figure.
class Seat {
public:
	explicit Seat(common::IDX index);

	Seat(const Seat&) = delete;
	Seat& operator=(const Seat&) = delete;

	Seat(Seat&&) noexcept = default;
	Seat& operator=(Seat&&) noexcept = default;

	[[nodiscard]] common::IDX getIndex() const;
	void setIndex(common::IDX index);

	[[nodiscard]] bool isFree() const;

	[[nodiscard]] std::shared_ptr<Player> getPlayer() const;
	void setPlayer(std::shared_ptr<Player> player);
	void clearPlayer();

	[[nodiscard]] common::ID getFigureTypeId() const;
	void setFigureTypeId(common::ID figureTypeId);

private:
	common::IDX m_index{};
	std::shared_ptr<Player> m_player{};
	common::ID m_figureTypeId{1};
};

}  // namespace botf
