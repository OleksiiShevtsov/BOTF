#include "engine/game_sessions_pool.h"

#include <gtest/gtest.h>

TEST(GameSessionsPool, EnsureDefaultForCreatesSharedSession) {
	botf::GameSessionsPool pool;
	EXPECT_TRUE(pool.empty());

	botf::GameSession& a = pool.ensureDefaultFor(1);
	EXPECT_EQ(pool.size(), 1u);
	EXPECT_EQ(a.getMapId(), 1u);
	EXPECT_EQ(pool.findGameSessionByClient(1), &a);

	botf::GameSession& b = pool.ensureDefaultFor(2);
	EXPECT_EQ(pool.size(), 1u);
	EXPECT_EQ(&a, &b);
	EXPECT_EQ(pool.findGameSessionByClient(1), pool.findGameSessionByClient(2));
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

	EXPECT_EQ(pool.findGameSessionByClient(1), nullptr);
	ASSERT_NE(pool.findGameSessionByClient(2), nullptr);

	const auto clients = pool.clientsOf(*pool.findGameSessionByClient(2));
	ASSERT_EQ(clients.size(), 1u);
	EXPECT_EQ(clients.front(), 2u);
}
