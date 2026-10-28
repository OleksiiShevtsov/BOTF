#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "helpers/test_env.h"
#include "logic/game_logic.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

float speedOf(const botf::FigureObject& figure) {
	const auto v = figure.getVelocity();
	return std::sqrt(v.x * v.x + v.y * v.y);
}

botf::common::ClientInput moveInput(int dx, int dy) {
	return botf::common::ClientInput{dx, dy, {0.f, 0.f}, false, 0};
}

}  // namespace

TEST(GameLogicMovement, SpeedClampedByVectorMagnitude) {
	botf::GameLogic logic;
	botf::test::spawnBound(logic, 1, {0.f, 0.f});
	botf::test::applyClientInput(logic, 1, moveInput(1, 0));

	for (int i = 0; i < 80; ++i) {
		logic.tick(kDt);
		EXPECT_LE(speedOf(logic.figures().front()),
				  botf::common::GlobalSettings::defaultPlayerSpeedLimit + 1e-3f);
	}
	EXPECT_NEAR(speedOf(logic.figures().front()),
				botf::common::GlobalSettings::defaultPlayerSpeedLimit, 1.f);
}

TEST(GameLogicMovement, DiagonalDoesNotExceedSpeedLimit) {
	botf::GameLogic logic;
	botf::test::spawnBound(logic, 1, {0.f, 0.f});
	botf::test::applyClientInput(logic, 1, moveInput(1, 1));

	for (int i = 0; i < 80; ++i) {
		logic.tick(kDt);
		EXPECT_LE(speedOf(logic.figures().front()),
				  botf::common::GlobalSettings::defaultPlayerSpeedLimit + 1e-3f);
	}
}

TEST(GameLogicMovement, ReleaseStopsWithoutReverse) {
	botf::GameLogic logic;
	botf::test::spawnBound(logic, 1, {0.f, 0.f});
	botf::test::applyClientInput(logic, 1, moveInput(1, 0));
	for (int i = 0; i < 40; ++i) {
		logic.tick(kDt);
	}

	const float xAtRelease = logic.figures().front().getPosition().x;
	ASSERT_GT(xAtRelease, 1.f);
	ASSERT_GT(logic.figures().front().getVelocity().x, 0.f);

	botf::test::applyClientInput(logic, 1, moveInput(0, 0));
	float prevX = xAtRelease;
	for (int i = 0; i < 80; ++i) {
		logic.tick(kDt);
		const float x = logic.figures().front().getPosition().x;
		EXPECT_GE(x, xAtRelease - 1e-3f);
		prevX = x;
	}
	EXPECT_NEAR(speedOf(logic.figures().front()), 0.f, 1e-3f);
	EXPECT_GE(prevX, xAtRelease - 1e-3f);
}

TEST(GameLogicMovement, DirectionChangeHasBoundedStep) {
	botf::GameLogic logic;
	botf::test::spawnBound(logic, 1, {0.f, 0.f});
	botf::test::applyClientInput(logic, 1, moveInput(1, 0));
	for (int i = 0; i < 40; ++i) {
		logic.tick(kDt);
	}

	botf::test::applyClientInput(logic, 1, moveInput(-1, 0));
	const float limit = botf::common::GlobalSettings::defaultPlayerSpeedLimit;
	const float maxStep = limit * kDt + 1.f;  // one tick of max speed + slack
	for (int i = 0; i < 10; ++i) {
		const auto before = logic.figures().front().getPosition();
		logic.tick(kDt);
		const auto after = logic.figures().front().getPosition();
		const float step =
			std::sqrt((after.x - before.x) * (after.x - before.x) +
					  (after.y - before.y) * (after.y - before.y));
		EXPECT_LE(step, maxStep) << "teleport-like step on direction change";
	}
}

TEST(GameLogicMovement, UpInputMovesNegativeGameY) {
	botf::GameLogic logic;
	botf::test::spawnBound(logic, 1, {0.f, 100.f});
	botf::test::applyClientInput(logic, 1, moveInput(0, +1));
	for (int i = 0; i < 20; ++i) {
		logic.tick(kDt);
	}
	EXPECT_LT(logic.figures().front().getPosition().y, 100.f - 1.f);
}
