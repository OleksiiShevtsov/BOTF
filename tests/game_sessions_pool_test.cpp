#include "helpers/test_env.h"
#include "utils/random.hpp"

#include <gtest/gtest.h>

#include <unordered_set>

TEST(GameSessionsPool, JoinFormingCreatesSessionAndSharesMap) {
	botf::test::TestSessionPool env;
	EXPECT_TRUE(env.pool.empty());

	const botf::common::ID idA = env.joinClient(1);
	EXPECT_EQ(env.pool.size(), 1u);
	EXPECT_EQ(idA, 1u);
	ASSERT_NE(env.sessionByClient(1), nullptr);
	EXPECT_EQ(env.sessionByClient(1)->phase(), botf::SessionPhase::Forming);
	EXPECT_EQ(env.sessionByClient(1)->getGameMap().getId(), 1u);
	EXPECT_EQ(&env.sessionByClient(1)->getGameMap(), env.mapPool.getGameMapById(1).get());
}

TEST(GameSessionsPool, JoinFormingIsIdempotentPerClient) {
	botf::test::TestSessionPool env;
	const botf::common::ID first = env.joinClient(7);
	const botf::common::ID again = env.joinClient(7);
	EXPECT_EQ(first, again);
	EXPECT_EQ(env.pool.size(), 1u);
	EXPECT_EQ(env.pool.getSessionClients(first).size(), 1u);
}

TEST(GameSessionsPool, TwoClientsShareOneFormingSession) {
	botf::test::TestSessionPool env;
	const botf::common::ID idA = env.joinClient(1);
	const botf::common::ID idB = env.joinClient(2);
	EXPECT_EQ(idA, idB);
	EXPECT_EQ(env.pool.size(), 1u);
	EXPECT_EQ(env.sessionByClient(1), env.sessionByClient(2));
	EXPECT_EQ(env.sessionByClient(1)->phase(), botf::SessionPhase::Forming);
}

TEST(GameSessionsPool, RemoveClientDropsMembership) {
	botf::test::TestSessionPool env;
	env.joinClient(1);
	env.joinClient(2);
	env.pool.removeClient(1);

	EXPECT_EQ(env.sessionByClient(1), nullptr);
	ASSERT_NE(env.sessionByClient(2), nullptr);

	const auto clients = env.pool.getSessionClients(env.sessionByClient(2)->getId());
	ASSERT_EQ(clients.size(), 1u);
	EXPECT_EQ(clients.front(), 2u);
}

TEST(GameSession, BeginPlayingSeatsAllClientsAndRefusesWrongCount) {
	botf::utils::seedRandom(1);
	botf::test::TestSession env;
	ASSERT_EQ(env.session.seatCapacity(), 2u);
	EXPECT_EQ(env.session.phase(), botf::SessionPhase::Forming);
	EXPECT_TRUE(env.session.hasFreeSeats(0));
	EXPECT_TRUE(env.session.hasFreeSeats(1));
	EXPECT_FALSE(env.session.hasFreeSeats(2));

	EXPECT_FALSE(env.session.beginPlaying({1}));
	EXPECT_EQ(env.session.phase(), botf::SessionPhase::Forming);
	EXPECT_TRUE(env.session.figures().empty());

	ASSERT_TRUE(env.session.beginPlaying({1, 2}));
	EXPECT_EQ(env.session.phase(), botf::SessionPhase::Playing);
	ASSERT_EQ(env.session.figures().size(), 2u);
	ASSERT_TRUE(env.session.entityForClient(1).has_value());
	ASSERT_TRUE(env.session.entityForClient(2).has_value());
	EXPECT_NE(*env.session.entityForClient(1), *env.session.entityForClient(2));

	const auto& spawns = env.session.getGameMap().getSpawns();
	std::unordered_set<float> seatedX;
	for (const auto& figure : env.session.figures()) {
		seatedX.insert(figure.getPosition().x);
	}
	EXPECT_EQ(seatedX.count(spawns[0].x), 1u);
	EXPECT_EQ(seatedX.count(spawns[1].x), 1u);

	EXPECT_FALSE(env.session.beginPlaying({1, 2}));
}

TEST(GameSessionsPool, ThirdClientOpensNewFormingAfterFullSessionStarted) {
	botf::utils::seedRandom(2);
	botf::test::TestSessionPool env;

	const botf::common::ID first = env.joinClient(1);
	EXPECT_EQ(env.joinClient(2), first);
	ASSERT_EQ(env.pool.getSessionClients(first).size(), 2u);

	botf::GameSession* session = env.sessionById(first);
	ASSERT_NE(session, nullptr);
	ASSERT_TRUE(session->beginPlaying(env.pool.getSessionClients(first)));
	EXPECT_EQ(session->phase(), botf::SessionPhase::Playing);
	EXPECT_FALSE(session->hasFreeSeats(env.pool.getSessionClients(first).size()));

	const botf::common::ID second = env.joinClient(3);
	EXPECT_NE(second, first);
	EXPECT_EQ(env.pool.size(), 2u);
	ASSERT_NE(env.sessionByClient(3), nullptr);
	EXPECT_EQ(env.sessionByClient(3)->phase(), botf::SessionPhase::Forming);
}
