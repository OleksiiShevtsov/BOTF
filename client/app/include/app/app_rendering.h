#pragma once

#include "app/input_handler.h"
#include "app/match.h"
#include "app/player.h"
#include "common/types.hpp"

#include <memory>

#include <SFML/Graphics.hpp>

namespace botf::app {

// Window + input + draw orchestration (Match owns map + scene).
class AppRendering {
public:
	AppRendering();

	AppRendering(const AppRendering&) = delete;
	AppRendering(AppRendering&&) = delete;

	[[nodiscard]] Player& getPlayer();
	[[nodiscard]] const Player& getPlayer() const;
	[[nodiscard]] InputHandler& getInputHandler();
	[[nodiscard]] const InputHandler& getInputHandler() const;
	[[nodiscard]] Match& getMatch();
	[[nodiscard]] const Match& getMatch() const;

	[[nodiscard]] bool isWindowOpen() const;
	// Window pixel coords (identity with world while camera is 1:1).
	[[nodiscard]] common::Point mouseWorldPosition() const;
	void draw();

private:
	Player m_player{};
	InputHandler m_inputHandler{};
	Match m_match{};
	std::shared_ptr<sf::RenderWindow> m_window;
};

}  // namespace botf::app
