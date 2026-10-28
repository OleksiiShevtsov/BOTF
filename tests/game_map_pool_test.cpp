#include "engine/game_map_pool.h"
#include "helpers/test_env.h"
#include "logic/game_map.h"
#include "utils/map_loader.hpp"

#include <gtest/gtest.h>

TEST(UtilsMapLoader, ListAndLoadMapOne) {
	const auto ids = botf::utils::listMapIds();
	ASSERT_FALSE(ids.empty());
	EXPECT_EQ(ids.front(), 1u);

	const auto data = botf::utils::loadMap(1);
	ASSERT_TRUE(data.has_value());
	EXPECT_EQ(data->id, 1u);
	EXPECT_EQ(data->name, "arena_stub");
	EXPECT_FLOAT_EQ(data->width, 1080.f);
	EXPECT_FLOAT_EQ(data->height, 640.f);
	ASSERT_EQ(data->spawns.size(), 2u);
	EXPECT_FLOAT_EQ(data->spawns[0].x, 100.f);
	EXPECT_FLOAT_EQ(data->spawns[0].y, 300.f);
	EXPECT_FLOAT_EQ(data->spawns[1].x, 980.f);
	EXPECT_FLOAT_EQ(data->spawns[1].y, 300.f);
}

TEST(GameMapPool, LoadLoadsGameMap) {
	botf::GameMapPool pool;
	ASSERT_TRUE(pool.load());
	EXPECT_EQ(pool.size(), 1u);

	const auto gameMap = pool.getGameMapById(1);
	ASSERT_NE(gameMap, nullptr);
	EXPECT_EQ(gameMap->getId(), 1u);
	EXPECT_EQ(gameMap->getName(), "arena_stub");
	EXPECT_FLOAT_EQ(gameMap->getWidth(), 1080.f);
	EXPECT_FLOAT_EQ(gameMap->getHeight(), 640.f);
	ASSERT_EQ(gameMap->getSpawns().size(), 2u);
}

TEST(GameMapPool, LoadPickRandomAndShare) {
	botf::GameMapPool pool;
	ASSERT_TRUE(pool.load());
	EXPECT_EQ(pool.size(), 1u);

	const auto first = pool.pickRandomGameMap();
	const auto second = pool.pickRandomGameMap();
	ASSERT_NE(first, nullptr);
	ASSERT_NE(second, nullptr);
	EXPECT_EQ(first->getId(), 1u);
	EXPECT_EQ(second->getId(), 1u);
	EXPECT_EQ(first, second);  // single map template instance in pool
	EXPECT_EQ(pool.getGameMapById(1), first);
}

TEST(GameSessionsPool, CreateSessionSharesMapFromPool) {
	botf::test::TestSessionPool env;
	const botf::common::ID sessionId = env.ensureClient(1);
	botf::GameSession* gameSession = env.sessionById(sessionId);
	ASSERT_NE(gameSession, nullptr);
	EXPECT_EQ(gameSession->getGameMap().getId(), 1u);
	EXPECT_FLOAT_EQ(gameSession->getGameMap().getWidth(), 1080.f);
	ASSERT_GE(gameSession->getGameMap().getSpawns().size(), 2u);

	const auto fromPool = env.mapPool.getGameMapById(1);
	ASSERT_NE(fromPool, nullptr);
	EXPECT_EQ(&gameSession->getGameMap(), fromPool.get());
}

TEST(GameSession, BindOrSpawnUsesFirstFreeSpawnsAndRefusesWhenFull) {
	botf::test::TestSession env;
	const auto& spawns = env.session.getGameMap().getSpawns();
	ASSERT_EQ(spawns.size(), 2u);

	const auto a = env.session.bindOrSpawnClient(1);
	ASSERT_TRUE(a.has_value());
	EXPECT_FLOAT_EQ(env.session.figures()[0].getPosition().x, spawns[0].x);
	EXPECT_FLOAT_EQ(env.session.figures()[0].getPosition().y, spawns[0].y);

	const auto b = env.session.bindOrSpawnClient(2);
	ASSERT_TRUE(b.has_value());
	EXPECT_NE(*a, *b);
	EXPECT_FLOAT_EQ(env.session.figures()[1].getPosition().x, spawns[1].x);
	EXPECT_FLOAT_EQ(env.session.figures()[1].getPosition().y, spawns[1].y);

	const auto refused = env.session.bindOrSpawnClient(3);
	EXPECT_FALSE(refused.has_value());
	EXPECT_EQ(env.session.figures().size(), 2u);

	env.session.seatPlayersRandom();
}
