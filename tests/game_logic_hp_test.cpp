#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "common/types.hpp"
#include "helpers/test_env.h"

#include <gtest/gtest.h>

namespace {

botf::common::ID spawnBound(botf::GameSession& session, botf::common::ID clientId) {
	const auto entityId = session.spawnPlayer(
		botf::common::Point{0.f, 0.f}, botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	session.bindClient(clientId, entityId);
	return entityId;
}

}  // namespace

TEST(GameLogicHp, SpawnStartsAliveAtMaxHp) {
	botf::test::TestSession env;
	auto& session = env.session;
	const auto id = session.spawnPlayer(botf::common::Point{10.f, 20.f},
										botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	ASSERT_EQ(session.figures().size(), 1u);
	EXPECT_EQ(session.figures().front().getId(), id);
	EXPECT_EQ(session.figures().front().getHp(), botf::common::GlobalSettings::maxHp);
	EXPECT_EQ(session.figures().front().getLifeStatus(), botf::common::LifeStatus::Alive);
	EXPECT_FALSE(session.figures().front().isDead());
}

TEST(GameLogicHp, DamageToZeroMarksDeadAndClamps) {
	botf::test::TestSession env;
	auto& session = env.session;
	const auto id = session.spawnPlayer(botf::common::Point{0.f, 0.f},
										botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	const int maxHp = botf::common::GlobalSettings::maxHp;
	const int damage = botf::common::GlobalSettings::bulletDamage;

	int hp = maxHp;
	while (hp > 0) {
		ASSERT_TRUE(session.applyDamage(id, damage));
		hp = session.figures().front().getHp();
		EXPECT_GE(hp, 0);
	}
	EXPECT_EQ(session.figures().front().getHp(), 0);
	EXPECT_EQ(session.figures().front().getLifeStatus(), botf::common::LifeStatus::Dead);
	EXPECT_TRUE(session.figures().front().isDead());
	EXPECT_EQ(session.figures().size(), 1u);

	ASSERT_TRUE(session.applyDamage(id, damage));
	EXPECT_EQ(session.figures().front().getHp(), 0);
	EXPECT_EQ(session.figures().size(), 1u);
}

TEST(GameLogicHp, DeadIgnoresMoveAndFire) {
	botf::test::TestSession env;
	auto& session = env.session;
	const botf::common::ID clientId = 1;
	const auto id = spawnBound(session, clientId);
	session.applyClientInput(clientId, botf::common::ClientInput{0, 0, {50.f, 0.f}, false, 0});

	const int damage = botf::common::GlobalSettings::bulletDamage;
	while (!session.figures().front().isDead()) {
		ASSERT_TRUE(session.applyDamage(id, damage));
	}

	session.applyClientInput(clientId, botf::common::ClientInput{1, 0, {50.f, 0.f}, false, 0});
	EXPECT_EQ(session.figures().front().inputDx(), 0);
	EXPECT_EQ(session.figures().front().inputDy(), 0);
	session.applyClientInput(clientId, botf::common::ClientInput{0, 0, {50.f, 0.f}, true, 0});
	EXPECT_TRUE(session.bullets().empty());
}

TEST(GameLogicHp, RemoveClientDestroysFigure) {
	botf::test::TestSession env;
	auto& session = env.session;
	const botf::common::ID clientId = 2;
	const auto id = spawnBound(session, clientId);
	EXPECT_EQ(session.entityForClient(clientId), id);
	session.removeClient(clientId);
	EXPECT_FALSE(session.entityForClient(clientId).has_value());
	EXPECT_TRUE(session.figures().empty());
}

TEST(GameLogicHp, ApplyDamageMissingEntityFails) {
	botf::test::TestSession env;
	EXPECT_FALSE(env.session.applyDamage(999, botf::common::GlobalSettings::bulletDamage));
}
