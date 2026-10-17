#pragma once

#include "app/i_view_object.h"

namespace botf::app {

// Visual + pose for one moving figure from WorldSnapshot.
// Not the local input Player (app/player.h).
class FigureView : public IViewObject {
public:
	explicit FigureView(common::ObjectID id);

	[[nodiscard]] common::ObjectID getId() const override;
	[[nodiscard]] common::Point getPosition() const override;
	[[nodiscard]] common::Angle getAngle() const override;

	void setPose(common::Point position, common::Angle angle) override;
	void draw(sf::RenderTarget& target) const override;

private:
	common::ObjectID m_id{};
	common::Point m_position{};
	common::Angle m_angle{};
};

}  // namespace botf::app
