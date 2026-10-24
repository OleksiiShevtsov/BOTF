#pragma once

#include "app/i_view_object.h"
#include "common/types.hpp"

namespace botf::app {

// Visual + pose for one moving figure from WorldSnapshot.
// Not the local input Player (app/player.h).
class FigureView : public IViewObject {
public:
	explicit FigureView(common::ID id);

	[[nodiscard]] common::ID getId() const override;
	[[nodiscard]] common::Point getPosition() const override;
	[[nodiscard]] common::Angle getAngle() const override;

	void setPose(common::Point position, common::Angle angle) override;
	void setLifeStatus(common::LifeStatus status);
	void draw(sf::RenderTarget& target) const override;

private:
	common::ID m_id{};
	common::Point m_position{};
	common::Angle m_angle{};
	common::LifeStatus m_lifeStatus{common::LifeStatus::Alive};
};

}  // namespace botf::app
