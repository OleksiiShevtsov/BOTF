#pragma once

#include "common/types.hpp"

#include <SFML/Graphics.hpp>

namespace botf::app {

// Client arena / map layer. Not an IViewObject — different role from movers.
class GameMap {
public:
	void setMapId(common::ID mapId);
	[[nodiscard]] common::ID getId() const;

	void draw(sf::RenderTarget& target) const;

private:
	common::ID m_mapId{1};
};

}  // namespace botf::app
