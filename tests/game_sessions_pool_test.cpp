#include "helpers/test_env.h"
#include "utils/random.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <unordered_set>

TEST(GameSessionsPool, JoinFormingCreatesSessionWithoutMapYet) {
	botf::test::TestSessionPool env;
	EXPECT_TRUE(env.pool.empty());

	const botf::common::ID idA = env.joinClient(1);
	EXPECT_EQ(env.pool.size(), 1u);
	EXPECT_EQ(idA, 1u);
	ASSERT_NE(env.sessionByClient(1), nullptr);
	EXPECT_EQ(env.sessionByClient(1)->getSessionState(), botf::SessionState::Forming);
	EXPECT_EQ(env.sessionByClient(1)->clientCount(), 1u);
	EXPECT_FALSE(env.sessionByClient(1)->hasGameMap());
	EXPECT_EQ(env.sessionByClient(1)->seatCapacity(), 2u);
}

TEST(GameSessionsPool, JoinFormingIsIdempotentPerClient) {
	botf::test::TestSessionPool env;
	const botf::common::ID first = env.joinClient(7);
	const botf::common::ID again = env.joinClient(7);
	EXPECT_EQ(first, again);
	EXPECT_EQ(env.pool.size(), 1u);
	EXPECT_EQ(env.sessionByClient(7)->clientCount(), 1u);
}

TEST(GameSessionsPool, TwoClientsShareOneFormingSession) {
	botf::test::TestSessionPool env;
	const botf::common::ID idA = env.joinClient(1);
	const botf::common::ID idB = env.joinClient(2);
	EXPECT_EQ(idA, idB);
	EXPECT_EQ(env.pool.size(), 1u);
	EXPECT_EQ(env.sessionByClient(1), env.sessionByClient(2));
	EXPECT_EQ(env.sessionByClient(1)->getSessionState(), botf::SessionState::Forming);
	EXPECT_EQ(env.sessionByClient(1)->clientCount(), 2u);
}

TEST(GameSessionsPool, RemovePlayerDropsMembership) {
	botf::test::TestSessionPool env;
	env.joinClient(1);
	env.joinClient(2);
	env.pool.removePlayer(1);

	EXPECT_EQ(env.sessionByClient(1), nullptr);
	ASSERT_NE(env.sessionByClient(2), nullptr);
	EXPECT_EQ(env.sessionByClient(2)->clientCount(), 1u);
	ASSERT_EQ(env.sessionByClient(2)->clients().size(), 1u);
	EXPECT_EQ(env.sessionByClient(2)->clients().front(), 2u);
}

TEST(GameSession, BeginPlayingSeatsAllClientsAndRefusesWrongCount) {
	botf::utils::seedRandom(1);
	botf::test::TestSession env;
	ASSERT_EQ(env.session.seatCapacity(), 2u);
	EXPECT_EQ(env.session.getSessionState(), botf::SessionState::Forming);
	EXPECT_TRUE(env.session.hasFreeSeats());
	EXPECT_FALSE(env.session.hasGameMap());

	EXPECT_FALSE(env.beginPlaying());
	EXPECT_EQ(env.session.getSessionState(), botf::SessionState::Forming);

	ASSERT_TRUE(env.session.addPlayer(std::make_shared<botf::Player>(1), 1));
	EXPECT_TRUE(env.session.hasFreeSeats());
	EXPECT_FALSE(env.beginPlaying());
	EXPECT_TRUE(env.session.figures().empty());

	ASSERT_TRUE(env.session.addPlayer(std::make_shared<botf::Player>(2), 1));
	EXPECT_FALSE(env.session.hasFreeSeats());
	EXPECT_FALSE(env.session.addPlayer(std::make_shared<botf::Player>(3), 1));

	ASSERT_TRUE(env.beginPlaying());
	EXPECT_EQ(env.session.getSessionState(), botf::SessionState::Playing);
	ASSERT_TRUE(env.session.hasGameMap());
	ASSERT_EQ(env.session.figures().size(), 2u);
	ASSERT_TRUE(env.session.hasClient(1));
	ASSERT_TRUE(env.session.hasClient(2));
	ASSERT_TRUE(env.session.figureIdForClient(1).has_value());
	ASSERT_TRUE(env.session.figureIdForClient(2).has_value());
	EXPECT_NE(*env.session.figureIdForClient(1), *env.session.figureIdForClient(2));

	// Seat.index is the figure id and spawn slot.
	for (const auto& seat : env.session.seats()) {
		ASSERT_TRUE(seat->getPlayer());
		EXPECT_EQ(seat->getFigureTypeId(), 1);
		const auto figureId = env.session.figureIdForClient(seat->getPlayer()->getId());
		ASSERT_TRUE(figureId.has_value());
		EXPECT_EQ(*figureId, static_cast<botf::common::ID>(seat->getIndex()));
		bool found = false;
		for (const auto& figure : env.session.figures()) {
			if (figure.getId() == *figureId) {
				found = true;
				EXPECT_FLOAT_EQ(
					figure.getPosition().x,
					env.session.getGameMap().getSpawns()[static_cast<std::size_t>(seat->getIndex())]
						.x);
				EXPECT_FLOAT_EQ(figure.getHitbox().getWidth(), 40.f);
				EXPECT_FLOAT_EQ(figure.getHitbox().getHeight(), 40.f);
				break;
			}
		}
		EXPECT_TRUE(found);
	}

	const auto& spawns = env.session.getGameMap().getSpawns();
	std::unordered_set<float> seatedX;
	for (const auto& figure : env.session.figures()) {
		seatedX.insert(figure.getPosition().x);
	}
	EXPECT_EQ(seatedX.count(spawns[0].x), 1u);
	EXPECT_EQ(seatedX.count(spawns[1].x), 1u);

	EXPECT_FALSE(env.beginPlaying());
}

TEST(GameSessionsPool, ThirdClientOpensNewFormingAfterFullSessionStarted) {
	botf::utils::seedRandom(2);
	botf::test::TestSessionPool env;

	const botf::common::ID first = env.joinClient(1);
	EXPECT_EQ(env.joinClient(2), first);
	ASSERT_EQ(env.sessionById(first)->clientCount(), 2u);

	botf::GameSession* session = env.sessionById(first);
	ASSERT_NE(session, nullptr);
	ASSERT_TRUE(env.beginPlaying(first));
	EXPECT_EQ(session->getSessionState(), botf::SessionState::Playing);
	EXPECT_FALSE(session->hasFreeSeats());

	const botf::common::ID second = env.joinClient(3);
	EXPECT_NE(second, first);
	EXPECT_EQ(env.pool.size(), 2u);
	ASSERT_NE(env.sessionByClient(3), nullptr);
	EXPECT_EQ(env.sessionByClient(3)->getSessionState(), botf::SessionState::Forming);
	EXPECT_EQ(env.sessionByClient(3)->clientCount(), 1u);
}
