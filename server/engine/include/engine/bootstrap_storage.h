#pragma once

#include "engine/figure_description_pool.h"
#include "engine/game_map_pool.h"

namespace botf {

// Content hub: map + figure templates. Loaded once at server start.
// Passed into GameSession::beginPlaying — not stored on the session.
class BootstrapStorage {
public:
	BootstrapStorage() = default;

	BootstrapStorage(const BootstrapStorage&) = delete;
	BootstrapStorage& operator=(const BootstrapStorage&) = delete;

	// Load maps and figures from content/. false if either catalog empty.
	bool load();

	[[nodiscard]] GameMapPool& gameMapPool();
	[[nodiscard]] const GameMapPool& gameMapPool() const;

	[[nodiscard]] FigureDescriptionPool& figureDescriptionPool();
	[[nodiscard]] const FigureDescriptionPool& figureDescriptionPool() const;

private:
	GameMapPool m_gameMapPool;
	FigureDescriptionPool m_figureDescriptionPool;
};

}  // namespace botf
