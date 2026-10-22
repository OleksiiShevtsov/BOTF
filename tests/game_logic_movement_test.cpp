#include "common/global_settings.hpp"
#include "logic/game_logic.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

float speedOf(const botf::FigureObject& figure) {
	const auto v = figure.getVelocity();
	return std::sqrt(v.x * v.x + v.y * v.y);
}

}  // namespace

TEST(GameLogicMovement, SpeedClampedByVectorMagnitude) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{0.f, 0.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(1, id);
	game.setClientInput(1, 1, 0);

	for (int i = 0; i < 80; ++i) {
		game.tick(kDt);
		EXPECT_LE(speedOf(game.figures().front()),
				  botf::common::GlobalSettings::defaultPlayerSpeedLimit + 1e-3f);
	}
	EXPECT_NEAR(speedOf(game.figures().front()),
				botf::common::GlobalSettings::defaultPlayerSpeedLimit, 1.f);
}

TEST(GameLogicMovement, DiagonalDoesNotExceedSpeedLimit) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{0.f, 0.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(1, id);
	game.setClientInput(1, 1, 1);

	for (int i = 0; i < 80; ++i) {
		game.tick(kDt);
		EXPECT_LE(speedOf(game.figures().front()),
				  botf::common::GlobalSettings::defaultPlayerSpeedLimit + 1e-3f);
	}
}

TEST(GameLogicMovement, ReleaseStopsWithoutReverse) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{0.f, 0.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(1, id);
	game.setClientInput(1, 1, 0);
	for (int i = 0; i < 40; ++i) {
		game.tick(kDt);
	}

	const float xAtRelease = game.figures().front().position().x;
	ASSERT_GT(xAtRelease, 1.f);
	ASSERT_GT(game.figures().front().getVelocity().x, 0.f);

	game.setClientInput(1, 0, 0);
	float prevX = xAtRelease;
	for (int i = 0; i < 80; ++i) {
		game.tick(kDt);
		const float x = game.figures().front().position().x;
		// May coast forward while braking, but must not reverse past release point meaningfully.
		EXPECT_GE(x, xAtRelease - 1e-3f);
		prevX = x;
	}
	EXPECT_NEAR(speedOf(game.figures().front()), 0.f, 1e-3f);
	EXPECT_GE(prevX, xAtRelease - 1e-3f);
}

TEST(GameLogicMovement, DirectionChangeHasBoundedStep) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{0.f, 0.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(1, id);
	game.setClientInput(1, 1, 0);
	for (int i = 0; i < 40; ++i) {
		game.tick(kDt);
	}

	game.setClientInput(1, -1, 0);
	const float limit = botf::common::GlobalSettings::defaultPlayerSpeedLimit;
	const float maxStep = limit * kDt + 1.f;	 // one tick of max speed + slack
	for (int i = 0; i < 10; ++i) {
		const auto before = game.figures().front().position();
		game.tick(kDt);
		const auto after = game.figures().front().position();
		const float step =
			std::sqrt((after.x - before.x) * (after.x - before.x) +
					  (after.y - before.y) * (after.y - before.y));
		EXPECT_LE(step, maxStep) << "teleport-like step on direction change";
	}
}

TEST(GameLogicMovement, UpInputMovesNegativeGameY) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{0.f, 100.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(1, id);
	game.setClientInput(1, 0, +1);
	for (int i = 0; i < 20; ++i) {
		game.tick(kDt);
	}
	EXPECT_LT(game.figures().front().position().y, 100.f - 1.f);
}
