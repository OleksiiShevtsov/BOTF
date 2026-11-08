#include "engine/bootstrap_storage.h"
#include "engine/figure_description_pool.h"
#include "engine/game_map_pool.h"
#include "helpers/test_env.h"
#include "logic/game_map.h"
#include "utils/figure_loader.hpp"
#include "utils/map_loader.hpp"
#include "utils/random.hpp"

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

TEST(UtilsFigureLoader, ListAndLoadFigureOne) {
	const auto ids = botf::utils::listFigureIds();
	ASSERT_FALSE(ids.empty());
	EXPECT_EQ(ids.front(), 1u);

	const auto data = botf::utils::loadFigure(1);
	ASSERT_TRUE(data.has_value());
	EXPECT_EQ(data->id, 1u);
	EXPECT_EQ(data->name, "default");
	EXPECT_FLOAT_EQ(data->hitboxWidth, 40.f);
	EXPECT_FLOAT_EQ(data->hitboxHeight, 40.f);
	EXPECT_FLOAT_EQ(data->hitboxOffset.x, 0.f);
	EXPECT_FLOAT_EQ(data->hitboxOffset.y, 0.f);
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

TEST(FigureDescriptionPool, LoadLoadsFigure) {
	botf::FigureDescriptionPool pool;
	ASSERT_TRUE(pool.load());
	EXPECT_EQ(pool.size(), 1u);
	EXPECT_EQ(pool.firstFigureTypeId(), 1u);

	const auto figure = pool.getById(1);
	ASSERT_NE(figure, nullptr);
	EXPECT_EQ(figure->getId(), 1u);
	EXPECT_EQ(figure->getName(), "default");
	EXPECT_FLOAT_EQ(figure->getHitbox().getWidth(), 40.f);
	EXPECT_FLOAT_EQ(figure->getHitbox().getHeight(), 40.f);
}

TEST(BootstrapStorage, LoadMapsAndFigures) {
	botf::BootstrapStorage storage;
	ASSERT_TRUE(storage.load());
	EXPECT_EQ(storage.gameMapPool().size(), 1u);
	EXPECT_EQ(storage.figureDescriptionPool().size(), 1u);
}

TEST(GameSessionsPool, BeginPlayingAssignsMapFromBootstrapStorage) {
	botf::utils::seedRandom(1);
	botf::test::TestSessionPool env;
	const botf::common::ID sessionId = env.joinClient(1);
	EXPECT_EQ(env.joinClient(2), sessionId);
	botf::GameSession* gameSession = env.sessionById(sessionId);
	ASSERT_NE(gameSession, nullptr);
	EXPECT_FALSE(gameSession->hasGameMap());

	ASSERT_TRUE(env.beginPlaying(sessionId));
	EXPECT_EQ(gameSession->getGameMap().getId(), 1u);
	EXPECT_FLOAT_EQ(gameSession->getGameMap().getWidth(), 1080.f);
	ASSERT_GE(gameSession->getGameMap().getSpawns().size(), 2u);

	const auto fromPool = env.storage->gameMapPool().getGameMapById(1);
	ASSERT_NE(fromPool, nullptr);
	EXPECT_EQ(&gameSession->getGameMap(), fromPool.get());
}
