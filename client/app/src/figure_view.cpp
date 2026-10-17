#include "app/figure_view.h"

#include <numbers>

namespace {

// Tip along local +Y; atan2(0)=+X → rotate by angle_deg - 90 so tip faces aim.
constexpr float kTipLocalX = 0.f;
constexpr float kTipLocalY = 20.f;
constexpr float kBaseLeftX = -20.f;
constexpr float kBaseLeftY = -20.f;
constexpr float kBaseRightX = 20.f;
constexpr float kBaseRightY = -20.f;

}  // namespace

botf::app::FigureView::FigureView(common::ObjectID id) : m_id(id) {}

botf::common::ObjectID botf::app::FigureView::getId() const {
	return m_id;
}

botf::common::Point botf::app::FigureView::getPosition() const {
	return m_position;
}

botf::common::Angle botf::app::FigureView::getAngle() const {
	return m_angle;
}

void botf::app::FigureView::setPose(common::Point position, common::Angle angle) {
	m_position = position;
	m_angle = angle;
}

void botf::app::FigureView::draw(sf::RenderTarget& target) const {
	sf::ConvexShape triangle;
	triangle.setPointCount(3);
#if SFML_VERSION_MAJOR >= 3
	triangle.setPoint(0, {kTipLocalX, kTipLocalY});
	triangle.setPoint(1, {kBaseLeftX, kBaseLeftY});
	triangle.setPoint(2, {kBaseRightX, kBaseRightY});
#else
	triangle.setPoint(0, sf::Vector2f(kTipLocalX, kTipLocalY));
	triangle.setPoint(1, sf::Vector2f(kBaseLeftX, kBaseLeftY));
	triangle.setPoint(2, sf::Vector2f(kBaseRightX, kBaseRightY));
#endif
	triangle.setFillColor(sf::Color(120, 20, 20));
#if SFML_VERSION_MAJOR >= 3
	triangle.setOrigin({0.f, 0.f});
	triangle.setPosition({m_position.x, m_position.y});
#else
	triangle.setOrigin(0.f, 0.f);
	triangle.setPosition(m_position.x, m_position.y);
#endif
	// Radians (logic) → degrees (SFML). Tip is +Y local; aim 0 is +X.
	const float degrees = m_angle * (180.f / std::numbers::pi_v<float>)-90.f;
	triangle.setRotation(degrees);
	target.draw(triangle);
}
