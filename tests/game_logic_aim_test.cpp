#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "helpers/test_env.h"
#include "logic/game_logic.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

}  // namespace

TEST(GameLogicAim, StoredAimPointRetargetsAngleAfterMove) {
	botf::GameLogic logic;
	const botf::common::ID clientId = 1;
	botf::test::spawnBound(logic, clientId, {100.f, 100.f});

	const botf::common::Point aim{200.f, 100.f};
	botf::test::applyClientInput(logic, clientId, botf::common::ClientInput{0, 0, aim, false, 0});
	EXPECT_NEAR(logic.figures().front().getAngle(), 0.f, 1e-4f);

	botf::test::applyClientInput(logic, clientId, botf::common::ClientInput{0, +1, aim, false, 0});
	for (int i = 0; i < 40; ++i) {
		logic.tick(kDt);
	}

	const auto pos = logic.figures().front().getPosition();
	ASSERT_LT(pos.y, 100.f - 5.f);

	const float expected = std::atan2(aim.y - pos.y, aim.x - pos.x);
	EXPECT_NEAR(logic.figures().front().getAngle(), expected, 1e-4f);
	EXPECT_GT(std::fabs(logic.figures().front().getAngle()), 0.1f);
}

TEST(GameLogicAim, UnboundClientAimIsIgnored) {
	botf::GameLogic logic;
	logic.spawnPlayer(botf::common::Point{0.f, 0.f});
	botf::test::applyClientInput(logic, 99, botf::common::ClientInput{0, 0, {50.f, 0.f}, false, 0});
	EXPECT_FLOAT_EQ(logic.figures().front().getAngle(), 0.f);
}
