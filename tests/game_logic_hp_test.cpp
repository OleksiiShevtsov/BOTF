#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "common/types.hpp"
#include "helpers/test_env.h"
#include "logic/game_logic.h"

#include <gtest/gtest.h>

TEST(GameLogicHp, SpawnStartsAliveAtMaxHp) {
	botf::GameLogic logic;
	const auto id = logic.spawnPlayer(botf::common::Point{10.f, 20.f});
	ASSERT_EQ(logic.figures().size(), 1u);
	EXPECT_EQ(logic.figures().front().getId(), id);
	EXPECT_EQ(logic.figures().front().getHp(), botf::common::GlobalSettings::maxHp);
	EXPECT_EQ(logic.figures().front().getLifeStatus(), botf::common::LifeStatus::Alive);
	EXPECT_FALSE(logic.figures().front().isDead());
}

TEST(GameLogicHp, DamageToZeroMarksDeadAndClamps) {
	botf::GameLogic logic;
	const auto id = logic.spawnPlayer(botf::common::Point{0.f, 0.f});
	const int maxHp = botf::common::GlobalSettings::maxHp;
	const int damage = botf::common::GlobalSettings::bulletDamage;

	int hp = maxHp;
	while (hp > 0) {
		ASSERT_TRUE(logic.applyDamage(id, damage));
		hp = logic.figures().front().getHp();
		EXPECT_GE(hp, 0);
	}
	EXPECT_EQ(logic.figures().front().getHp(), 0);
	EXPECT_EQ(logic.figures().front().getLifeStatus(), botf::common::LifeStatus::Dead);
	EXPECT_TRUE(logic.figures().front().isDead());
	EXPECT_EQ(logic.figures().size(), 1u);

	ASSERT_TRUE(logic.applyDamage(id, damage));
	EXPECT_EQ(logic.figures().front().getHp(), 0);
	EXPECT_EQ(logic.figures().size(), 1u);
}

TEST(GameLogicHp, DeadIgnoresMoveAndFire) {
	botf::GameLogic logic;
	const botf::common::ID clientId = 1;
	const auto id = botf::test::spawnBound(logic, clientId);
	botf::test::applyClientInput(logic, clientId, botf::common::ClientInput{0, 0, {50.f, 0.f}, false, 0});

	const int damage = botf::common::GlobalSettings::bulletDamage;
	while (!logic.figures().front().isDead()) {
		ASSERT_TRUE(logic.applyDamage(id, damage));
	}

	botf::test::applyClientInput(logic, clientId, botf::common::ClientInput{1, 0, {50.f, 0.f}, false, 0});
	EXPECT_EQ(logic.figures().front().inputDx(), 0);
	EXPECT_EQ(logic.figures().front().inputDy(), 0);
	botf::test::applyClientInput(logic, clientId, botf::common::ClientInput{0, 0, {50.f, 0.f}, true, 0});
	EXPECT_TRUE(logic.bullets().empty());
}

TEST(GameLogicHp, RemoveClientDestroysFigure) {
	botf::GameLogic logic;
	const botf::common::ID clientId = 2;
	const auto id = botf::test::spawnBound(logic, clientId);
	EXPECT_EQ(logic.entityForClient(clientId), id);
	logic.removeClient(clientId);
	EXPECT_FALSE(logic.entityForClient(clientId).has_value());
	EXPECT_TRUE(logic.figures().empty());
}

TEST(GameLogicHp, ApplyDamageMissingEntityFails) {
	botf::GameLogic logic;
	EXPECT_FALSE(logic.applyDamage(999, botf::common::GlobalSettings::bulletDamage));
}
