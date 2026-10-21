#pragma once

#include <SFML/Graphics.hpp>

namespace botf::app {

// TODO: draft HUD — replace with art / layout system later (cards 32–33).
// Draws local player info (HP bar) in screen space.
class InformationPanel {
public:
	void setHp(int currentHp, int maxHp);

	void draw(sf::RenderTarget& target) const;

private:
	int m_currentHp{0};
	int m_maxHp{1};
};

}  // namespace botf::app
