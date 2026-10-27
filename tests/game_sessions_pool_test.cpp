#include "helpers/test_env.h"

#include <gtest/gtest.h>

TEST(GameSessionsPool, EnsureDefaultForCreatesSharedSession) {
	botf::test::TestSessionPool env;
	EXPECT_TRUE(env.pool.empty());

	const botf::common::ID idA = env.ensureClient(1);
	EXPECT_EQ(env.pool.size(), 1u);
	EXPECT_EQ(idA, 1u);
	ASSERT_NE(env.sessionByClient(1), nullptr);
	EXPECT_EQ(env.sessionByClient(1)->getId(), idA);
	EXPECT_EQ(env.sessionById(idA), env.sessionByClient(1));

	const botf::common::ID idB = env.ensureClient(2);
	EXPECT_EQ(env.pool.size(), 1u);
	EXPECT_EQ(idA, idB);
	EXPECT_EQ(env.sessionByClient(1), env.sessionByClient(2));
}

TEST(GameSessionsPool, EnsureDefaultForIsIdempotentPerClient) {
	botf::test::TestSessionPool env;
	const botf::common::ID first = env.ensureClient(7);
	const botf::common::ID again = env.ensureClient(7);
	EXPECT_EQ(first, again);
	EXPECT_EQ(env.pool.size(), 1u);
}

TEST(GameSessionsPool, RemoveClientDropsMembership) {
	botf::test::TestSessionPool env;
	env.ensureClient(1);
	env.ensureClient(2);
	env.pool.removeClient(1);

	EXPECT_EQ(env.sessionByClient(1), nullptr);
	ASSERT_NE(env.sessionByClient(2), nullptr);

	const auto clients = env.pool.getSessionClients(env.sessionByClient(2)->getId());
	ASSERT_EQ(clients.size(), 1u);
	EXPECT_EQ(clients.front(), 2u);
}
