#include "app/information_panel.h"

#include <algorithm>

namespace {

constexpr float kMargin = 20.f;
constexpr float kBackdropWidth = 104.f;
constexpr float kBackdropHeight = 14.f;
constexpr float kFillMaxWidth = 100.f;
constexpr float kFillHeight = 10.f;

}  // namespace

void botf::app::InformationPanel::setHp(int currentHp, int maxHp) {
	m_currentHp = std::max(0, currentHp);
	m_maxHp = std::max(1, maxHp);
}

void botf::app::InformationPanel::draw(sf::RenderTarget& target) const {
	const auto windowSize = target.getSize();
	const float windowW = static_cast<float>(windowSize.x);
	const float windowH = static_cast<float>(windowSize.y);

	const float backdropX = windowW - kMargin - kBackdropWidth;
	const float backdropY = windowH - kMargin - kBackdropHeight;

	sf::RectangleShape backdrop;
#if SFML_VERSION_MAJOR >= 3
	backdrop.setSize({kBackdropWidth, kBackdropHeight});
	backdrop.setPosition({backdropX, backdropY});
#else
	backdrop.setSize(sf::Vector2f(kBackdropWidth, kBackdropHeight));
	backdrop.setPosition(backdropX, backdropY);
#endif
	backdrop.setFillColor(sf::Color::Black);
	target.draw(backdrop);

	const float ratio =
		static_cast<float>(m_currentHp) / static_cast<float>(m_maxHp);
	const float fillWidth = kFillMaxWidth * std::clamp(ratio, 0.f, 1.f);
	if (fillWidth <= 0.f) {
		return;
	}

	const float fillX = backdropX + (kBackdropWidth - kFillMaxWidth) * 0.5f;
	const float fillY = backdropY + (kBackdropHeight - kFillHeight) * 0.5f;

	sf::RectangleShape fill;
#if SFML_VERSION_MAJOR >= 3
	fill.setSize({fillWidth, kFillHeight});
	fill.setPosition({fillX, fillY});
#else
	fill.setSize(sf::Vector2f(fillWidth, kFillHeight));
	fill.setPosition(fillX, fillY);
#endif
	fill.setFillColor(sf::Color(0, 180, 0));
	target.draw(fill);
}
