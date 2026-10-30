#pragma once

#include "common/types.hpp"

#include <SFML/Graphics.hpp>

namespace botf::app {

// Full-screen menu: solid background + centered "START GAME" button (card 45).
class MenuScreen {
public:
	MenuScreen();

	MenuScreen(const MenuScreen&) = delete;
	MenuScreen& operator=(const MenuScreen&) = delete;

	[[nodiscard]] bool isReady() const;
	[[nodiscard]] bool containsPoint(common::Point point) const;
	void setHovered(bool hovered);
	void draw(sf::RenderTarget& target) const;

private:
	sf::Font m_font{};
	sf::RectangleShape m_button{};
	sf::Text m_label{};
	bool m_fontLoaded = false;
	bool m_hovered = false;
};

}  // namespace botf::app
