#pragma once

#include "app/input_handler.h"
#include "app/match.h"
#include "app/menu_screen.h"
#include "app/player.h"
#include "common/types.hpp"

#include <memory>

#include <SFML/Graphics.hpp>

namespace botf::app {

// Window + input + screen presenters. Flow ownership stays in ClientApp.
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
	[[nodiscard]] MenuScreen& getMenuScreen();
	[[nodiscard]] const MenuScreen& getMenuScreen() const;

	[[nodiscard]] bool isWindowOpen() const;
	// Window pixel coords (identity with world while camera is 1:1).
	[[nodiscard]] common::Point mouseWorldPosition() const;

	void drawMenu();
	void drawWaiting();
	void drawMatch();

private:
	void beginFrame();
	void endFrame();

	Player m_player{};
	InputHandler m_inputHandler{};
	Match m_match{};
	MenuScreen m_menuScreen{};
	std::shared_ptr<sf::RenderWindow> m_window;
};

}  // namespace botf::app
