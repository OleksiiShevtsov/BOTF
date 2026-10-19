#include "app/bullet_view.h"

#include "common/global_settings.hpp"

#include <numbers>

botf::app::BulletView::BulletView(common::ObjectID id) : m_id(id) {}

botf::common::ObjectID botf::app::BulletView::getId() const {
	return m_id;
}

botf::common::Point botf::app::BulletView::getPosition() const {
	return m_position;
}

botf::common::Angle botf::app::BulletView::getAngle() const {
	return m_angle;
}

void botf::app::BulletView::setPose(common::Point position, common::Angle angle) {
	m_position = position;
	m_angle = angle;
}

void botf::app::BulletView::draw(sf::RenderTarget& target) const {
	const float length = common::GlobalSettings::bulletVisualLength;
	const float width = common::GlobalSettings::bulletVisualWidth;

	sf::RectangleShape segment;
#if SFML_VERSION_MAJOR >= 3
	segment.setSize({length, width});
	segment.setOrigin({length * 0.5f, width * 0.5f});
	segment.setPosition({m_position.x, m_position.y});
#else
	segment.setSize(sf::Vector2f(length, width));
	segment.setOrigin(length * 0.5f, width * 0.5f);
	segment.setPosition(m_position.x, m_position.y);
#endif
	segment.setFillColor(sf::Color(220, 180, 40));
	const float degrees = m_angle * (180.f / std::numbers::pi_v<float>);
#if SFML_VERSION_MAJOR >= 3
	segment.setRotation(sf::degrees(degrees));
#else
	segment.setRotation(degrees);
#endif
	target.draw(segment);
}
