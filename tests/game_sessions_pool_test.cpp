#include "engine/game_sessions_pool.h"

#include <gtest/gtest.h>

TEST(GameSessionsPool, EnsureDefaultForCreatesSharedSession) {
	botf::GameSessionsPool pool;
	EXPECT_TRUE(pool.empty());

	botf::GameSession& a = pool.ensureDefaultFor(1);
	EXPECT_EQ(pool.size(), 1u);
	EXPECT_EQ(a.getMapId(), 1u);
	EXPECT_TRUE(a.contains(1));

	botf::GameSession& b = pool.ensureDefaultFor(2);
	EXPECT_EQ(pool.size(), 1u);
	EXPECT_EQ(&a, &b);
	EXPECT_TRUE(b.contains(2));
	EXPECT_EQ(pool.findByClient(1), pool.findByClient(2));
}

TEST(GameSessionsPool, EnsureDefaultForIsIdempotentPerClient) {
	botf::GameSessionsPool pool;
	botf::GameSession& first = pool.ensureDefaultFor(7);
	botf::GameSession& again = pool.ensureDefaultFor(7);
	EXPECT_EQ(&first, &again);
	EXPECT_EQ(pool.size(), 1u);
}

TEST(GameSessionsPool, RemoveClientDropsMembership) {
	botf::GameSessionsPool pool;
	pool.ensureDefaultFor(1);
	pool.ensureDefaultFor(2);
	pool.removeClient(1);

	EXPECT_EQ(pool.findByClient(1), nullptr);
	ASSERT_NE(pool.findByClient(2), nullptr);
	EXPECT_FALSE(pool.findByClient(2)->contains(1));
	EXPECT_TRUE(pool.findByClient(2)->contains(2));
}
