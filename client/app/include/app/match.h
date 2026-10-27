#pragma once

#include "app/game_map.h"
#include "app/information_panel.h"
#include "app/scene.h"
#include "common/dto.hpp"
#include "common/types.hpp"

#include <optional>

#include <SFML/Graphics.hpp>

namespace botf::app {

// Client-side match: owns map + scene, lifecycle hooks for GameState later.
class Match {
public:
	void start();
	void update();
	void finish();

	void applyWorldSnapshot(const common::WorldSnapshot& snapshot,
							std::optional<common::ID> selfId = std::nullopt);
	void draw(sf::RenderTarget& target) const;

	[[nodiscard]] GameMap& getMap();
	[[nodiscard]] const GameMap& getMap() const;
	[[nodiscard]] Scene& getScene();
	[[nodiscard]] const Scene& getScene() const;
	[[nodiscard]] const InformationPanel& getInformationPanel() const;

private:
	GameMap m_map{};
	Scene m_scene{};
	InformationPanel m_infoPanel{};
	bool m_running = false;
};

}  // namespace botf::app
