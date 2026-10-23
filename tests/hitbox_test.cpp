#include "common/global_settings.hpp"
#include "logic/game_logic.h"
#include "logic/hitbox.h"

#include <gtest/gtest.h>

namespace {

constexpr float kHalf = 20.f;  // hitboxSize 40 → half-extent 20

}  // namespace

TEST(Hitbox, HalfExtentMatchesGlobalSettings) {
	EXPECT_FLOAT_EQ(botf::hitboxHalfExtent(),
					botf::common::GlobalSettings::hitboxSize * 0.5f);
	EXPECT_FLOAT_EQ(botf::hitboxHalfExtent(), kHalf);
}

TEST(Hitbox, PointInsideAndOnBoundaryIsHit) {
	const botf::common::Point centerHitbox{100.f, 200.f};

	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, centerHitbox));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f + kHalf, 200.f}));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f - kHalf, 200.f}));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f, 200.f + kHalf}));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f, 200.f - kHalf}));
	// Triangle vertices from card 16 sit on the AABB edge.
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f, 200.f + kHalf}));		   // (0,+20)
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f - kHalf, 200.f - kHalf})); // (-20,-20)
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f + kHalf, 200.f - kHalf})); // (20,-20)
}

TEST(Hitbox, PointJustOutsideIsMiss) {
	const botf::common::Point centerHitbox{0.f, 0.f};
	constexpr float kEps = 0.01f;

	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, {kHalf + kEps, 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, {-(kHalf + kEps), 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, {0.f, kHalf + kEps}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, {0.f, -(kHalf + kEps)}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, {kHalf + kEps, kHalf + kEps}));
}

TEST(Hitbox, SegmentCrossingAabbIsHit) {
	const botf::common::Point centerHitbox{0.f, 0.f};
	// Passes through the square horizontally.
	EXPECT_TRUE(botf::segmentHitsAabb(centerHitbox, {-50.f, 0.f}, {50.f, 0.f}));
	// Starts outside, ends inside.
	EXPECT_TRUE(botf::segmentHitsAabb(centerHitbox, {-50.f, 0.f}, {0.f, 0.f}));
	// Touches a corner.
	EXPECT_TRUE(
		botf::segmentHitsAabb(centerHitbox, {kHalf, kHalf}, {kHalf + 10.f, kHalf + 10.f}));
}

TEST(Hitbox, SegmentMissingAabbIsMiss) {
	const botf::common::Point centerHitbox{0.f, 0.f};
	EXPECT_FALSE(botf::segmentHitsAabb(centerHitbox, {30.f, 30.f}, {40.f, 40.f}));
	EXPECT_FALSE(botf::segmentHitsAabb(centerHitbox, {-50.f, 25.f}, {50.f, 25.f}));
}

TEST(Hitbox, DegenerateSegmentIsPointTest) {
	const botf::common::Point centerHitbox{10.f, 10.f};
	const botf::common::Point pointHitInside{10.f, 10.f};
	const botf::common::Point pointHitOutside{10.f + kHalf + 1.f, 10.f};
	EXPECT_TRUE(botf::segmentHitsAabb(centerHitbox, pointHitInside, pointHitInside));
	EXPECT_FALSE(botf::segmentHitsAabb(centerHitbox, pointHitOutside, pointHitOutside));
}

// Acceptance smoke (card 34): bullet point vs figure center AABB — no damage yet (41).
TEST(Hitbox, BulletPointVsFigureCenter) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{100.f, 100.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	ASSERT_EQ(game.figures().size(), 1u);
	EXPECT_EQ(game.figures().front().getObjectId(), id);
	const botf::common::Point centerHitbox = game.figures().front().getPosition();

	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f, 100.f}));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, {100.f + kHalf, 100.f}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, {100.f + kHalf + 0.01f, 100.f}));
}
