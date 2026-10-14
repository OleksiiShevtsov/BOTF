#pragma once

#include "app/types.h"

#include <vector>

#include <SFML/Graphics.hpp>

namespace botf::app {

// All moving client-side objects (players now; projectiles later).
class ObjectDraw {
public:
	void clear();
	void setObjects(std::vector<SceneObject> objects);
	void setObject(common::ID id, common::Point position, common::Angle angle);

	void draw(sf::RenderTarget& target) const;

private:
	std::vector<SceneObject> m_objects;
};

}  // namespace botf::app
