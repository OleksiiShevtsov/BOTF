#pragma once

#include "app/i_view_object.h"

namespace botf::app {

// Visual + pose for one player from WorldSnapshot.
// Not the local input Player (app/player.h).
class PlayerView : public IViewObject {
public:
	explicit PlayerView(common::ID id);

	[[nodiscard]] common::ID id() const override;
	[[nodiscard]] common::Point position() const override;
	[[nodiscard]] common::Angle angle() const override;

	void setPose(common::Point position, common::Angle angle) override;
	void draw(sf::RenderTarget& target) const override;

private:
	common::ID m_id{};
	common::Point m_position{};
	common::Angle m_angle{};
};

}  // namespace botf::app
