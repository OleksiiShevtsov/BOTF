#include "app/menu_screen.h"

#include "common/global_settings.hpp"
#include "utils/path.hpp"

#include <SFML/Config.hpp>

#include <filesystem>

namespace {

constexpr float kButtonWidth = 280.f;
constexpr float kButtonHeight = 56.f;
// Slightly above geometric center of the window.
constexpr float kButtonCenterYOffset = -40.f;
constexpr unsigned int kLabelCharacterSize = 22;
constexpr unsigned int kWaitingCharacterSize = 28;

const sf::Color kButtonFill{55, 55, 55};
const sf::Color kButtonHoverFill{70, 70, 70};
const sf::Color kLabelFill{255, 255, 255};

void centerText(sf::Text& text, const float centerX, const float centerY) {
	const sf::FloatRect bounds = text.getLocalBounds();
#if SFML_VERSION_MAJOR >= 3
	text.setOrigin({bounds.size.x * 0.5f, bounds.size.y * 0.5f});
	text.setPosition({centerX, centerY});
#else
	text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
	text.setPosition(centerX, centerY);
#endif
}

}  // namespace

botf::app::MenuScreen::MenuScreen()
#if SFML_VERSION_MAJOR >= 3
	: m_label(m_font), m_waitingLabel(m_font)
#endif
{
	const std::filesystem::path fontPath = utils::contentRoot() / "ui" / "DejaVuSans.ttf";
#if SFML_VERSION_MAJOR >= 3
	m_fontLoaded = m_font.openFromFile(fontPath.string());
#else
	m_fontLoaded = m_font.loadFromFile(fontPath.string());
#endif
	if (!m_fontLoaded) {
		return;
	}

	const float windowW = static_cast<float>(common::GlobalSettings::sizeWindowX);
	const float windowH = static_cast<float>(common::GlobalSettings::sizeWindowY);
	const float buttonX = (windowW - kButtonWidth) * 0.5f;
	const float buttonY = (windowH - kButtonHeight) * 0.5f + kButtonCenterYOffset;

	m_button.setSize(sf::Vector2f{kButtonWidth, kButtonHeight});
	m_button.setPosition(sf::Vector2f{buttonX, buttonY});
	m_button.setFillColor(kButtonFill);

#if SFML_VERSION_MAJOR < 3
	m_label.setFont(m_font);
	m_waitingLabel.setFont(m_font);
#endif
	m_label.setString("START GAME");
	m_label.setCharacterSize(kLabelCharacterSize);
	m_label.setFillColor(kLabelFill);
	centerText(m_label, buttonX + kButtonWidth * 0.5f, buttonY + kButtonHeight * 0.5f);

	m_waitingLabel.setString("WAITING...");
	m_waitingLabel.setCharacterSize(kWaitingCharacterSize);
	m_waitingLabel.setFillColor(kLabelFill);
	centerText(m_waitingLabel, windowW * 0.5f, windowH * 0.5f + kButtonCenterYOffset);
}

bool botf::app::MenuScreen::isReady() const {
	return m_fontLoaded;
}

bool botf::app::MenuScreen::containsPoint(const common::Point point) const {
	const sf::FloatRect bounds = m_button.getGlobalBounds();
#if SFML_VERSION_MAJOR >= 3
	return bounds.contains(sf::Vector2f{point.x, point.y});
#else
	return bounds.contains(point.x, point.y);
#endif
}

void botf::app::MenuScreen::setHovered(const bool hovered) {
	m_hovered = hovered;
	m_button.setFillColor(m_hovered ? kButtonHoverFill : kButtonFill);
}

void botf::app::MenuScreen::draw(sf::RenderTarget& target) const {
	if (!m_fontLoaded) {
		return;
	}
	target.draw(m_button);
	target.draw(m_label);
}

void botf::app::MenuScreen::drawWaiting(sf::RenderTarget& target) const {
	if (!m_fontLoaded) {
		return;
	}
	target.draw(m_waitingLabel);
}
