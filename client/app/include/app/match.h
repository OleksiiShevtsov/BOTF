#pragma once

#include "app/game_map.h"
#include "app/scene.h"
#include "common/dto.hpp"

#include <SFML/Graphics.hpp>

namespace botf::app {

// Client-side match: owns map + scene, lifecycle hooks for GameState later.
class Match {
public:
	void start();
	void update();
	void finish();

	void applyWorldSnapshot(const common::WorldSnapshot& snapshot);
	void draw(sf::RenderTarget& target) const;

	[[nodiscard]] GameMap& getMap();
	[[nodiscard]] const GameMap& getMap() const;
	[[nodiscard]] Scene& getScene();
	[[nodiscard]] const Scene& getScene() const;

private:
	GameMap m_map{};
	Scene m_scene{};
	bool m_running = false;
};

}  // namespace botf::app
