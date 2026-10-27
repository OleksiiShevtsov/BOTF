#include "common/global_settings.hpp"
#include "logic/game_logic.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

}  // namespace

TEST(GameLogicFire, SpawnBulletFliesAndDiesAtMaxRange) {
	botf::GameLogic game;
	const botf::common::ID clientId = 1;
	const auto entityId = game.spawnPlayer(botf::common::Point{100.f, 100.f},
										   botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(clientId, entityId);
	game.setClientAim(clientId, botf::common::Point{200.f, 100.f});
	EXPECT_NEAR(game.figures().front().getAngle(), 0.f, 1e-4f);

	ASSERT_TRUE(game.tryFire(clientId));
	ASSERT_EQ(game.bullets().size(), 1u);
	EXPECT_EQ(game.bullets().front().getOwnerId(), entityId);

	const float maxRange = botf::common::GlobalSettings::bulletMaxRange;
	const float speed = botf::common::GlobalSettings::bulletSpeed;
	const int ticksNeeded = static_cast<int>(std::ceil(maxRange / (speed * kDt))) + 2;
	for (int i = 0; i < ticksNeeded; ++i) {
		game.tick(kDt);
	}
	EXPECT_TRUE(game.bullets().empty()) << "bullet should be destroyed past max range";
}

TEST(GameLogicFire, FireCooldownBlocksSecondShot) {
	botf::GameLogic game;
	const botf::common::ID clientId = 1;
	const auto entityId = game.spawnPlayer(botf::common::Point{0.f, 0.f},
										   botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(clientId, entityId);
	game.setClientAim(clientId, botf::common::Point{50.f, 0.f});

	ASSERT_TRUE(game.tryFire(clientId));
	EXPECT_FALSE(game.tryFire(clientId));
	EXPECT_EQ(game.bullets().size(), 1u);

	const float cooldown = botf::common::GlobalSettings::fireCooldownSeconds;
	const int waitTicks = static_cast<int>(std::ceil(cooldown / kDt)) + 1;
	for (int i = 0; i < waitTicks; ++i) {
		game.tick(kDt);
	}
	// First bullet may still be in flight (maxRange can outlive cooldown).
	const auto bulletsBefore = game.bullets().size();
	ASSERT_TRUE(game.tryFire(clientId));
	EXPECT_EQ(game.bullets().size(), bulletsBefore + 1u);
}

TEST(GameLogicFire, UnboundClientCannotFire) {
	botf::GameLogic game;
	game.spawnPlayer(botf::common::Point{0.f, 0.f},
					 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	EXPECT_FALSE(game.tryFire(99));
	EXPECT_TRUE(game.bullets().empty());
}
