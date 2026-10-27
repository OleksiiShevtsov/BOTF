#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "helpers/test_env.h"

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
	botf::test::TestSession env;
	auto& session = env.session;
	const auto id = session.spawnPlayer(botf::common::Point{0.f, 0.f});
	session.bindClient(1, id);
	session.applyClientInput(1, moveInput(1, 0));

	for (int i = 0; i < 80; ++i) {
		session.tick(kDt);
		EXPECT_LE(speedOf(session.figures().front()),
				  botf::common::GlobalSettings::defaultPlayerSpeedLimit + 1e-3f);
	}
	EXPECT_NEAR(speedOf(session.figures().front()),
				botf::common::GlobalSettings::defaultPlayerSpeedLimit, 1.f);
}

TEST(GameLogicMovement, DiagonalDoesNotExceedSpeedLimit) {
	botf::test::TestSession env;
	auto& session = env.session;
	const auto id = session.spawnPlayer(botf::common::Point{0.f, 0.f});
	session.bindClient(1, id);
	session.applyClientInput(1, moveInput(1, 1));

	for (int i = 0; i < 80; ++i) {
		session.tick(kDt);
		EXPECT_LE(speedOf(session.figures().front()),
				  botf::common::GlobalSettings::defaultPlayerSpeedLimit + 1e-3f);
	}
}

TEST(GameLogicMovement, ReleaseStopsWithoutReverse) {
	botf::test::TestSession env;
	auto& session = env.session;
	const auto id = session.spawnPlayer(botf::common::Point{0.f, 0.f});
	session.bindClient(1, id);
	session.applyClientInput(1, moveInput(1, 0));
	for (int i = 0; i < 40; ++i) {
		session.tick(kDt);
	}

	const float xAtRelease = session.figures().front().getPosition().x;
	ASSERT_GT(xAtRelease, 1.f);
	ASSERT_GT(session.figures().front().getVelocity().x, 0.f);

	session.applyClientInput(1, moveInput(0, 0));
	float prevX = xAtRelease;
	for (int i = 0; i < 80; ++i) {
		session.tick(kDt);
		const float x = session.figures().front().getPosition().x;
		EXPECT_GE(x, xAtRelease - 1e-3f);
		prevX = x;
	}
	EXPECT_NEAR(speedOf(session.figures().front()), 0.f, 1e-3f);
	EXPECT_GE(prevX, xAtRelease - 1e-3f);
}

TEST(GameLogicMovement, DirectionChangeHasBoundedStep) {
	botf::test::TestSession env;
	auto& session = env.session;
	const auto id = session.spawnPlayer(botf::common::Point{0.f, 0.f});
	session.bindClient(1, id);
	session.applyClientInput(1, moveInput(1, 0));
	for (int i = 0; i < 40; ++i) {
		session.tick(kDt);
	}

	session.applyClientInput(1, moveInput(-1, 0));
	const float limit = botf::common::GlobalSettings::defaultPlayerSpeedLimit;
	const float maxStep = limit * kDt + 1.f;  // one tick of max speed + slack
	for (int i = 0; i < 10; ++i) {
		const auto before = session.figures().front().getPosition();
		session.tick(kDt);
		const auto after = session.figures().front().getPosition();
		const float step =
			std::sqrt((after.x - before.x) * (after.x - before.x) +
					  (after.y - before.y) * (after.y - before.y));
		EXPECT_LE(step, maxStep) << "teleport-like step on direction change";
	}
}

TEST(GameLogicMovement, UpInputMovesNegativeGameY) {
	botf::test::TestSession env;
	auto& session = env.session;
	const auto id = session.spawnPlayer(botf::common::Point{0.f, 100.f});
	session.bindClient(1, id);
	session.applyClientInput(1, moveInput(0, +1));
	for (int i = 0; i < 20; ++i) {
		session.tick(kDt);
	}
	EXPECT_LT(session.figures().front().getPosition().y, 100.f - 1.f);
}
