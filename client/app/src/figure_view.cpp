#include "app/figure_view.h"

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
	sf::CircleShape circle(10.f);
	circle.setFillColor(sf::Color(120, 20, 20));
#if SFML_VERSION_MAJOR >= 3
	circle.setOrigin({10.f, 10.f});
	circle.setPosition({m_position.x, m_position.y});
#else
	circle.setOrigin(10.f, 10.f);
	circle.setPosition(m_position.x, m_position.y);
#endif
	// angle reserved for “nose” draw (card 16).
	(void)m_angle;
	target.draw(circle);
}
