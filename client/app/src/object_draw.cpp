#include "app/object_draw.h"

void botf::app::ObjectDraw::clear() {
	m_objects.clear();
}

void botf::app::ObjectDraw::setObjects(std::vector<SceneObject> objects) {
	m_objects = std::move(objects);
}

void botf::app::ObjectDraw::setObject(common::ID id, common::Point position, common::Angle angle) {
	for (auto& obj : m_objects) {
		if (obj.id == id) {
			obj.position = position;
			obj.angle = angle;
			return;
		}
	}
	m_objects.push_back(SceneObject{id, position, angle});
}

void botf::app::ObjectDraw::draw(sf::RenderTarget& target) const {
	for (const auto& el : m_objects) {
		sf::CircleShape circle(10.f);
		circle.setFillColor(sf::Color(120, 20, 20));
#if SFML_VERSION_MAJOR >= 3
		circle.setOrigin({10.f, 10.f});
		circle.setPosition({el.position.x, el.position.y});
#else
		circle.setOrigin(10.f, 10.f);
		circle.setPosition(el.position.x, el.position.y);
#endif
		// angle reserved for “nose” draw (card 16).
		(void)el.angle;
		target.draw(circle);
	}
}
