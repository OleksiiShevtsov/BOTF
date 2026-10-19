#pragma once

#include "common/types.hpp"

#include <SFML/Graphics.hpp>

namespace botf::app {

// Base for moving client-side view objects (FigureView, BulletView, …).
// draw() is one capability — state lives on the object.
class IViewObject {
public:
	virtual ~IViewObject() = default;

	[[nodiscard]] virtual common::ObjectID getId() const = 0;
	[[nodiscard]] virtual common::Point getPosition() const = 0;
	[[nodiscard]] virtual common::Angle getAngle() const = 0;

	virtual void setPose(common::Point position, common::Angle angle) = 0;
	virtual void draw(sf::RenderTarget& target) const = 0;
};

}  // namespace botf::app
