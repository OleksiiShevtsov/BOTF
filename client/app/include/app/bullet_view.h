#pragma once

#include "app/i_view_object.h"

namespace botf::app {

// Temporary visual for a server bullet (point on wire → short segment on client).
class BulletView : public IViewObject {
public:
	explicit BulletView(common::ObjectID id);

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
