#include "app/input_handler.h"

#include <SFML/Window/Keyboard.hpp>

botf::app::MoveInput botf::app::InputHandler::readMoveInput() const {
	MoveInput input{};

	const bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
					sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
	const bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ||
					  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
	const bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
					  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
	const bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
					   sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);

	if (up) {
		input.dy += 1;
	}
	if (down) {
		input.dy -= 1;
	}
	if (right) {
		input.dx += 1;
	}
	if (left) {
		input.dx -= 1;
	}

	// Opposite keys cancel to 0; keep in {-1,0,1}.
	if (input.dx > 1) {
		input.dx = 1;
	}
	if (input.dx < -1) {
		input.dx = -1;
	}
	if (input.dy > 1) {
		input.dy = 1;
	}
	if (input.dy < -1) {
		input.dy = -1;
	}

	return input;
}
