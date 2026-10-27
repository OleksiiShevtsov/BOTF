#include "engine/game_map.h"
#include "engine/game_map_pool.h"
#include "helpers/test_env.h"
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
	ASSERT_GE(data->spawns.size(), 2u);
	EXPECT_FLOAT_EQ(data->spawns[0].x, 100.f);
	EXPECT_FLOAT_EQ(data->spawns[0].y, 300.f);
	EXPECT_FLOAT_EQ(data->spawns[1].x, 980.f);
	EXPECT_FLOAT_EQ(data->spawns[1].y, 300.f);
}

TEST(GameMap, LoadFromContent) {
	const botf::GameMap map = botf::GameMap::load(1);
	EXPECT_EQ(map.getId(), 1u);
	EXPECT_EQ(map.getName(), "arena_stub");
	EXPECT_FLOAT_EQ(map.getWidth(), 1080.f);
	EXPECT_FLOAT_EQ(map.getHeight(), 640.f);
	ASSERT_EQ(map.getSpawns().size(), 2u);
}

TEST(GameMapPool, RefreshPickAndGet) {
	botf::GameMapPool pool;
	ASSERT_TRUE(pool.refresh());
	EXPECT_EQ(pool.size(), 1u);
	EXPECT_FALSE(pool.mapIds().empty());

	const botf::common::ID first = pool.pickNextMapId();
	const botf::common::ID second = pool.pickNextMapId();
	EXPECT_EQ(first, 1u);
	EXPECT_EQ(second, 1u);  // single map → always 1

	const botf::GameMap& map = pool.getMap(first);
	EXPECT_EQ(map.getId(), 1u);
	EXPECT_FLOAT_EQ(map.getWidth(), 1080.f);
}

TEST(GameSessionsPool, CreateSessionGetsMapCopy) {
	botf::test::TestSessionPool env;
	const botf::common::ID sessionId = env.ensureClient(1);
	botf::GameSession* gameSession = env.sessionById(sessionId);
	ASSERT_NE(gameSession, nullptr);
	EXPECT_EQ(gameSession->getGameMap().getId(), 1u);
	EXPECT_FLOAT_EQ(gameSession->getGameMap().getWidth(), 1080.f);
	ASSERT_GE(gameSession->getGameMap().getSpawns().size(), 2u);
}

TEST(GameSession, BindOrSpawnUsesMapSpawns) {
	botf::test::TestSession env;
	const auto a = env.session.bindOrSpawnClient(1);
	const auto b = env.session.bindOrSpawnClient(2);
	ASSERT_EQ(env.session.figures().size(), 2u);

	const auto& spawns = env.session.getGameMap().getSpawns();
	EXPECT_FLOAT_EQ(env.session.figures()[0].getPosition().x, spawns[0].x);
	EXPECT_FLOAT_EQ(env.session.figures()[0].getPosition().y, spawns[0].y);
	EXPECT_FLOAT_EQ(env.session.figures()[1].getPosition().x, spawns[1].x);
	EXPECT_FLOAT_EQ(env.session.figures()[1].getPosition().y, spawns[1].y);
	EXPECT_NE(a, b);
}
