#include "common/global_settings.hpp"
#include "logic/game_logic.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

}  // namespace

TEST(GameLogicAim, StoredAimPointRetargetsAngleAfterMove) {
	botf::GameLogic game;
	const botf::common::ClientID clientId = 1;
	const auto entityId = game.spawnPlayer(botf::common::Point{100.f, 100.f},
										   botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(clientId, entityId);

	// Fixed world aim to the right of spawn → angle 0 at apply time.
	const botf::common::Point aim{200.f, 100.f};
	game.setClientAim(clientId, aim);
	EXPECT_NEAR(game.figures().front().angle(), 0.f, 1e-4f);

	// Move without a new Aim message. Angle must follow the stored point.
	game.setClientInput(clientId, 0, +1);
	for (int i = 0; i < 40; ++i) {
		game.tick(kDt);
	}

	const auto pos = game.figures().front().position();
	ASSERT_LT(pos.y, 100.f - 5.f);

	const float expected = std::atan2(aim.y - pos.y, aim.x - pos.x);
	EXPECT_NEAR(game.figures().front().angle(), expected, 1e-4f);
	// Frozen-angle bug would keep 0; retarget must leave that.
	EXPECT_GT(std::fabs(game.figures().front().angle()), 0.1f);
}

TEST(GameLogicAim, UnboundClientAimIsIgnored) {
	botf::GameLogic game;
	game.spawnPlayer(botf::common::Point{0.f, 0.f},
					 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.setClientAim(99, botf::common::Point{50.f, 0.f});
	EXPECT_FLOAT_EQ(game.figures().front().angle(), 0.f);
}
