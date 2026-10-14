#include "app/app_rendering.h"

#include "common/global_settings.hpp"

botf::app::AppRendering::AppRendering()
#if SFML_VERSION_MAJOR >= 3
	: m_window{std::make_shared<sf::RenderWindow>(
		  sf::VideoMode({static_cast<unsigned int>(common::GlobalSettings::sizeWindowX),
						 static_cast<unsigned int>(common::GlobalSettings::sizeWindowY)}),
		  "Battle Of The Flatland")} {
#else
	: m_window{std::make_shared<sf::RenderWindow>(
		  sf::VideoMode(static_cast<unsigned int>(common::GlobalSettings::sizeWindowX),
						static_cast<unsigned int>(common::GlobalSettings::sizeWindowY)),
		  "Battle Of The Flatland")} {
#endif
	m_window->setFramerateLimit(60);
}

botf::app::Player& botf::app::AppRendering::player() {
	return m_player;
}

const botf::app::Player& botf::app::AppRendering::player() const {
	return m_player;
}

botf::app::InputHandler& botf::app::AppRendering::inputHandler() {
	return m_inputHandler;
}

const botf::app::InputHandler& botf::app::AppRendering::inputHandler() const {
	return m_inputHandler;
}

botf::app::Match& botf::app::AppRendering::match() {
	return m_match;
}

const botf::app::Match& botf::app::AppRendering::match() const {
	return m_match;
}

bool botf::app::AppRendering::isWindowOpen() const {
	return m_window && m_window->isOpen();
}

void botf::app::AppRendering::draw() {
	if (!m_window) {
		return;
	}

#if SFML_VERSION_MAJOR >= 3
	while (const std::optional event = m_window->pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			m_window->close();
		}
	}
#else
	sf::Event event{};
	while (m_window->pollEvent(event)) {
		if (event.type == sf::Event::Closed) {
			m_window->close();
		}
	}
#endif

	m_window->clear(sf::Color(25, 25, 25));
	m_match.draw(*m_window);
	m_window->display();
}
