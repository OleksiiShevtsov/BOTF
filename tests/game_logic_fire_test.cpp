#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "helpers/test_env.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

botf::common::ClientInput aimFire(botf::common::Point aim, bool fire) {
	return botf::common::ClientInput{0, 0, aim, fire, 0};
}

}  // namespace

TEST(GameLogicFire, SpawnBulletFliesAndDiesAtMaxRange) {
	botf::test::TestSession env;
	auto& session = env.session;
	const botf::common::ID clientId = 1;
	const auto entityId = session.spawnPlayer(
		botf::common::Point{100.f, 100.f}, botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	session.bindClient(clientId, entityId);
	session.applyClientInput(clientId, aimFire({200.f, 100.f}, false));
	EXPECT_NEAR(session.figures().front().getAngle(), 0.f, 1e-4f);

	session.applyClientInput(clientId, aimFire({200.f, 100.f}, true));
	ASSERT_EQ(session.bullets().size(), 1u);
	EXPECT_EQ(session.bullets().front().getOwnerId(), entityId);

	const float maxRange = botf::common::GlobalSettings::bulletMaxRange;
	const float speed = botf::common::GlobalSettings::bulletSpeed;
	const int ticksNeeded = static_cast<int>(std::ceil(maxRange / (speed * kDt))) + 2;
	for (int i = 0; i < ticksNeeded; ++i) {
		session.tick(kDt);
	}
	EXPECT_TRUE(session.bullets().empty()) << "bullet should be destroyed past max range";
}

TEST(GameLogicFire, FireCooldownBlocksSecondShot) {
	botf::test::TestSession env;
	auto& session = env.session;
	const botf::common::ID clientId = 1;
	const auto entityId = session.spawnPlayer(
		botf::common::Point{0.f, 0.f}, botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	session.bindClient(clientId, entityId);
	session.applyClientInput(clientId, aimFire({50.f, 0.f}, false));

	session.applyClientInput(clientId, aimFire({50.f, 0.f}, true));
	EXPECT_EQ(session.bullets().size(), 1u);
	session.applyClientInput(clientId, aimFire({50.f, 0.f}, true));
	EXPECT_EQ(session.bullets().size(), 1u);

	const float cooldown = botf::common::GlobalSettings::fireCooldownSeconds;
	const int waitTicks = static_cast<int>(std::ceil(cooldown / kDt)) + 1;
	for (int i = 0; i < waitTicks; ++i) {
		session.tick(kDt);
	}
	const auto bulletsBefore = session.bullets().size();
	session.applyClientInput(clientId, aimFire({50.f, 0.f}, true));
	EXPECT_EQ(session.bullets().size(), bulletsBefore + 1u);
}

TEST(GameLogicFire, UnboundClientCannotFire) {
	botf::test::TestSession env;
	auto& session = env.session;
	session.spawnPlayer(botf::common::Point{0.f, 0.f},
						botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	session.applyClientInput(99, aimFire({50.f, 0.f}, true));
	EXPECT_TRUE(session.bullets().empty());
}
