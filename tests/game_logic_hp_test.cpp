#include "common/global_settings.hpp"
#include "common/types.hpp"
#include "logic/game_logic.h"

#include <gtest/gtest.h>

namespace {

botf::common::ID spawnBound(botf::GameLogic& game, botf::common::ID clientId) {
	const auto entityId = game.spawnPlayer(botf::common::Point{0.f, 0.f},
										   botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(clientId, entityId);
	return entityId;
}

}  // namespace

TEST(GameLogicHp, SpawnStartsAliveAtMaxHp) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{10.f, 20.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	ASSERT_EQ(game.figures().size(), 1u);
	EXPECT_EQ(game.figures().front().getId(), id);
	EXPECT_EQ(game.figures().front().getHp(), botf::common::GlobalSettings::maxHp);
	EXPECT_EQ(game.figures().front().getLifeStatus(), botf::common::LifeStatus::Alive);
	EXPECT_FALSE(game.figures().front().isDead());
}

TEST(GameLogicHp, DamageToZeroMarksDeadAndClamps) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{0.f, 0.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	const int maxHp = botf::common::GlobalSettings::maxHp;
	const int damage = botf::common::GlobalSettings::bulletDamage;

	int hp = maxHp;
	while (hp > 0) {
		ASSERT_TRUE(game.applyDamage(id, damage));
		hp = game.figures().front().getHp();
		EXPECT_GE(hp, 0);
	}
	EXPECT_EQ(game.figures().front().getHp(), 0);
	EXPECT_EQ(game.figures().front().getLifeStatus(), botf::common::LifeStatus::Dead);
	EXPECT_TRUE(game.figures().front().isDead());
	EXPECT_EQ(game.figures().size(), 1u);

	ASSERT_TRUE(game.applyDamage(id, damage));
	EXPECT_EQ(game.figures().front().getHp(), 0);
	EXPECT_EQ(game.figures().size(), 1u);
}

TEST(GameLogicHp, DeadIgnoresMoveAndFire) {
	botf::GameLogic game;
	const botf::common::ID clientId = 1;
	const auto id = spawnBound(game, clientId);
	game.setClientAim(clientId, botf::common::Point{50.f, 0.f});

	const int damage = botf::common::GlobalSettings::bulletDamage;
	while (!game.figures().front().isDead()) {
		ASSERT_TRUE(game.applyDamage(id, damage));
	}

	game.setClientInput(clientId, 1, 0);
	EXPECT_EQ(game.figures().front().inputDx(), 0);
	EXPECT_EQ(game.figures().front().inputDy(), 0);
	EXPECT_FALSE(game.tryFire(clientId));
	EXPECT_TRUE(game.bullets().empty());
}

TEST(GameLogicHp, RemoveClientDestroysFigure) {
	botf::GameLogic game;
	const botf::common::ID clientId = 2;
	const auto id = spawnBound(game, clientId);
	EXPECT_EQ(game.entityForClient(clientId), id);
	game.removeClient(clientId);
	EXPECT_FALSE(game.entityForClient(clientId).has_value());
	EXPECT_TRUE(game.figures().empty());
}

TEST(GameLogicHp, ApplyDamageMissingEntityFails) {
	botf::GameLogic game;
	EXPECT_FALSE(game.applyDamage(999, botf::common::GlobalSettings::bulletDamage));
}
