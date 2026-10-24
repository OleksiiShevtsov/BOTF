#pragma once

#include "common/types.hpp"

#include <SFML/Graphics.hpp>

namespace botf::app {

// Client arena / map layer. Not an IViewObject — different role from movers.
class GameMap {
public:
	void setId(common::ID id);
	[[nodiscard]] common::ID getId() const;

	void draw(sf::RenderTarget& target) const;

private:
	common::ID m_id{1};
};

}  // namespace botf::app
