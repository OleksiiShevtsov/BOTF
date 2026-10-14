#pragma once

#include "common/types.hpp"

#include <SFML/Graphics.hpp>

namespace botf::app {

// Client-side map / arena layer. Visual only — no collision logic.
class MapDraw {
public:
	void setMapId(common::ID mapId);
	[[nodiscard]] common::ID mapId() const;

	void draw(sf::RenderTarget& target) const;

private:
	common::ID m_mapId{1};
};

}  // namespace botf::app
