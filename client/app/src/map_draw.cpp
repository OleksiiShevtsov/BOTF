#include "app/map_draw.h"

#include "common/global_settings.hpp"

void botf::app::MapDraw::setMapId(common::ID mapId) {
	m_mapId = mapId;
}

botf::common::ID botf::app::MapDraw::mapId() const {
	return m_mapId;
}

void botf::app::MapDraw::draw(sf::RenderTarget& target) const {
	// MVP: one default arena. mapId reserved for alternate visuals later.
	(void)m_mapId;

	const float width = static_cast<float>(common::GlobalSettings::sizeWindowX);
	const float height = static_cast<float>(common::GlobalSettings::sizeWindowY);
	constexpr float kMargin = 24.f;

	sf::RectangleShape field;
#if SFML_VERSION_MAJOR >= 3
	field.setSize({width - 2.f * kMargin, height - 2.f * kMargin});
	field.setPosition({kMargin, kMargin});
#else
	field.setSize(sf::Vector2f(width - 2.f * kMargin, height - 2.f * kMargin));
	field.setPosition(kMargin, kMargin);
#endif
	field.setFillColor(sf::Color(40, 44, 52));
	field.setOutlineThickness(2.f);
	field.setOutlineColor(sf::Color(90, 98, 110));
	target.draw(field);
}
