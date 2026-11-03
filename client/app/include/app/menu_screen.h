#pragma once

#include "common/types.hpp"

#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>

namespace botf::app {

// Menu button + Waiting label (shared font). Card 45/47.
class MenuScreen {
public:
	MenuScreen();

	MenuScreen(const MenuScreen&) = delete;
	MenuScreen& operator=(const MenuScreen&) = delete;

	[[nodiscard]] bool isReady() const;
	[[nodiscard]] bool containsPoint(common::Point point) const;
	void setHovered(bool hovered);
	void draw(sf::RenderTarget& target) const;
	void drawWaiting(sf::RenderTarget& target) const;

private:
	// Font before Text: SFML 3 Text must be constructed with a Font&.
	sf::Font m_font{};
	sf::RectangleShape m_button{};
#if SFML_VERSION_MAJOR >= 3
	sf::Text m_label;
	sf::Text m_waitingLabel;
#else
	sf::Text m_label{};
	sf::Text m_waitingLabel{};
#endif
	bool m_fontLoaded = false;
	bool m_hovered = false;
};

}  // namespace botf::app
