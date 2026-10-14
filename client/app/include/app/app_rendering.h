#pragma once

#include "app/input_handler.h"
#include "app/player.h"
#include "app/types.h"

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>

namespace botf::app {

class AppRendering {
public:
	AppRendering();

	AppRendering(const AppRendering&) = delete;
	AppRendering(AppRendering&&) = delete;

	[[nodiscard]] Player& player();
	[[nodiscard]] const Player& player() const;
	[[nodiscard]] InputHandler& inputHandler();
	[[nodiscard]] const InputHandler& inputHandler() const;

	// Scene for this frame (typically filled from WorldState).
	void clearScene();
	void setSceneObject(common::ID id, common::Point position, common::Angle angle);
	void setScene(std::vector<SceneObject> objects);

	[[nodiscard]] bool isWindowOpen() const;
	void draw();

private:
	void drawMap();
	void drawSceneObjects();

	Player m_player{};
	InputHandler m_inputHandler{};
	std::vector<SceneObject> m_scene;
	std::shared_ptr<sf::RenderWindow> m_window;
};

}  // namespace botf::app
