#include "engine/game_map_pool.h"
#include "helpers/test_env.h"
#include "logic/game_map.h"
#include "utils/map_loader.hpp"

#include <gtest/gtest.h>

namespace {

bool isMapSpawn(const botf::GameMap& map, const botf::common::Point& pos) {
	for (const auto& spawn : map.getSpawns()) {
		if (spawn.x == pos.x && spawn.y == pos.y) {
			return true;
		}
	}
	return false;
}

}  // namespace

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
	EXPECT_EQ(data->maxNumberPlayers, 2u);
	ASSERT_EQ(data->spawns.size(), data->maxNumberPlayers);
	EXPECT_FLOAT_EQ(data->spawns[0].x, 100.f);
	EXPECT_FLOAT_EQ(data->spawns[0].y, 300.f);
	EXPECT_FLOAT_EQ(data->spawns[1].x, 980.f);
	EXPECT_FLOAT_EQ(data->spawns[1].y, 300.f);
}

TEST(GameMapPool, RefreshLoadsGameMap) {
	botf::GameMapPool pool;
	ASSERT_TRUE(pool.refresh());
	EXPECT_EQ(pool.size(), 1u);

	const auto gameMap = pool.getGameMapById(1);
	ASSERT_NE(gameMap, nullptr);
	EXPECT_EQ(gameMap->getId(), 1u);
	EXPECT_EQ(gameMap->getName(), "arena_stub");
	EXPECT_FLOAT_EQ(gameMap->getWidth(), 1080.f);
	EXPECT_FLOAT_EQ(gameMap->getHeight(), 640.f);
	EXPECT_EQ(gameMap->getMaxNumberPlayers(), 2u);
	ASSERT_EQ(gameMap->getSpawns().size(), 2u);
}

TEST(GameMapPool, RefreshPickRandomAndShare) {
	botf::GameMapPool pool;
	ASSERT_TRUE(pool.refresh());
	EXPECT_EQ(pool.size(), 1u);

	const botf::common::ID first = pool.pickRandomMapId();
	const botf::common::ID second = pool.pickRandomMapId();
	EXPECT_EQ(first, 1u);
	EXPECT_EQ(second, 1u);  // single map → always 1

	const auto gameMap = pool.getGameMapById(first);
	ASSERT_NE(gameMap, nullptr);
	EXPECT_EQ(pool.findGameMapById(first), gameMap);
}

TEST(GameSessionsPool, CreateSessionSharesMapFromPool) {
	botf::test::TestSessionPool env;
	const botf::common::ID sessionId = env.ensureClient(1);
	botf::GameSession* gameSession = env.sessionById(sessionId);
	ASSERT_NE(gameSession, nullptr);
	EXPECT_EQ(gameSession->getGameMap().getId(), 1u);
	EXPECT_FLOAT_EQ(gameSession->getGameMap().getWidth(), 1080.f);
	ASSERT_GE(gameSession->getGameMap().getSpawns().size(), 2u);

	const auto fromPool = env.pool.getGameMapPool().getGameMapById(1);
	EXPECT_EQ(gameSession->getGameMapPtr(), fromPool);
}

TEST(GameSession, BindOrSpawnUsesMapSpawns) {
	botf::test::TestSession env;
	const auto a = env.session.bindOrSpawnClient(1);
	const auto b = env.session.bindOrSpawnClient(2);
	ASSERT_EQ(env.session.figures().size(), 2u);
	EXPECT_NE(a, b);

	const auto& map = env.session.getGameMap();
	EXPECT_TRUE(isMapSpawn(map, env.session.figures()[0].getPosition()));
	EXPECT_TRUE(isMapSpawn(map, env.session.figures()[1].getPosition()));

	// Stub exists for card 47 (no-op until Playing seating).
	env.session.seatPlayersRandom();
}
