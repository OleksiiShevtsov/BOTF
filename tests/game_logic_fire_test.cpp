#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "helpers/test_env.h"
#include "logic/game_logic.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

botf::common::ClientInput aimFire(botf::common::Point aim, bool fire) {
	return botf::common::ClientInput{0, 0, aim, fire, 0};
}

}  // namespace

TEST(GameLogicFire, SpawnBulletFliesAndDiesAtMaxRange) {
	botf::GameLogic logic;
	const auto entityId = botf::test::spawnFigure(logic, {100.f, 100.f});
	botf::test::applyEntityInput(logic, entityId, aimFire({200.f, 100.f}, false));
	EXPECT_NEAR(logic.figures().front().getAngle(), 0.f, 1e-4f);

	botf::test::applyEntityInput(logic, entityId, aimFire({200.f, 100.f}, true));
	ASSERT_EQ(logic.bullets().size(), 1u);
	EXPECT_EQ(logic.bullets().front().getOwnerId(), entityId);

	const float maxRange = botf::common::GlobalSettings::bulletMaxRange;
	const float speed = botf::common::GlobalSettings::bulletSpeed;
	const int ticksNeeded = static_cast<int>(std::ceil(maxRange / (speed * kDt))) + 2;
	for (int i = 0; i < ticksNeeded; ++i) {
		logic.tick(kDt);
	}
	EXPECT_TRUE(logic.bullets().empty()) << "bullet should be destroyed past max range";
}

TEST(GameLogicFire, FireCooldownBlocksSecondShot) {
	botf::GameLogic logic;
	const auto entityId = botf::test::spawnFigure(logic, {0.f, 0.f});
	botf::test::applyEntityInput(logic, entityId, aimFire({50.f, 0.f}, false));

	botf::test::applyEntityInput(logic, entityId, aimFire({50.f, 0.f}, true));
	EXPECT_EQ(logic.bullets().size(), 1u);
	botf::test::applyEntityInput(logic, entityId, aimFire({50.f, 0.f}, true));
	EXPECT_EQ(logic.bullets().size(), 1u);

	const float cooldown = botf::common::GlobalSettings::fireCooldownSeconds;
	const int waitTicks = static_cast<int>(std::ceil(cooldown / kDt)) + 1;
	for (int i = 0; i < waitTicks; ++i) {
		logic.tick(kDt);
	}
	const auto bulletsBefore = logic.bullets().size();
	botf::test::applyEntityInput(logic, entityId, aimFire({50.f, 0.f}, true));
	EXPECT_EQ(logic.bullets().size(), bulletsBefore + 1u);
}

TEST(GameLogicFire, MissingEntityCannotFire) {
	botf::GameLogic logic;
	ASSERT_TRUE(logic.spawnFigure(0, botf::common::Point{0.f, 0.f}));
	botf::test::applyEntityInput(logic, 99, aimFire({50.f, 0.f}, true));
	EXPECT_TRUE(logic.bullets().empty());
}
