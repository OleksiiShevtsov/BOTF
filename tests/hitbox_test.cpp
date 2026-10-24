#include "common/global_settings.hpp"
#include "logic/game_logic.h"
#include "logic/hitbox.h"

#include <gtest/gtest.h>

namespace {

constexpr float kHalf = 20.f;  // hitboxSize 40 → half-extent 20
const botf::ObjectHitbox kDefaultBox = botf::installObjectHitbox();

}  // namespace

TEST(Hitbox, DefaultDescMatchesGlobalSettings) {
	EXPECT_FLOAT_EQ(kDefaultBox.width, botf::common::GlobalSettings::hitboxSize);
	EXPECT_FLOAT_EQ(kDefaultBox.height, botf::common::GlobalSettings::hitboxSize);
	EXPECT_FLOAT_EQ(kDefaultBox.getHalfWidth(), kHalf);
	EXPECT_FLOAT_EQ(kDefaultBox.getHalfHeight(), kHalf);
}

TEST(Hitbox, PointInsideAndOnBoundaryIsHit) {
	const botf::common::Point centerHitbox{100.f, 200.f};

	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, kDefaultBox, centerHitbox));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {100.f + kHalf, 200.f}));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {100.f - kHalf, 200.f}));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {100.f, 200.f + kHalf}));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {100.f, 200.f - kHalf}));
	// Triangle vertices from card 16 sit on the AABB edge.
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {100.f, 200.f + kHalf}));  // (0,+20)
	EXPECT_TRUE(
		botf::pointHitsAabb(centerHitbox, kDefaultBox, {100.f - kHalf, 200.f - kHalf}));  // (-20,-20)
	EXPECT_TRUE(
		botf::pointHitsAabb(centerHitbox, kDefaultBox, {100.f + kHalf, 200.f - kHalf}));  // (20,-20)
}

TEST(Hitbox, PointJustOutsideIsMiss) {
	const botf::common::Point centerHitbox{0.f, 0.f};
	constexpr float kEps = 0.01f;

	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {kHalf + kEps, 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {-(kHalf + kEps), 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {0.f, kHalf + kEps}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {0.f, -(kHalf + kEps)}));
	EXPECT_FALSE(botf::pointHitsAabb(centerHitbox, kDefaultBox, {kHalf + kEps, kHalf + kEps}));
}

TEST(Hitbox, SegmentCrossingAabbIsHit) {
	const botf::common::Point centerHitbox{0.f, 0.f};
	EXPECT_TRUE(botf::segmentHitsAabb(centerHitbox, kDefaultBox, {-50.f, 0.f}, {50.f, 0.f}));
	EXPECT_TRUE(botf::segmentHitsAabb(centerHitbox, kDefaultBox, {-50.f, 0.f}, {0.f, 0.f}));
	EXPECT_TRUE(botf::segmentHitsAabb(centerHitbox, kDefaultBox, {kHalf, kHalf},
									  {kHalf + 10.f, kHalf + 10.f}));
}

TEST(Hitbox, SegmentMissingAabbIsMiss) {
	const botf::common::Point centerHitbox{0.f, 0.f};
	EXPECT_FALSE(botf::segmentHitsAabb(centerHitbox, kDefaultBox, {30.f, 30.f}, {40.f, 40.f}));
	EXPECT_FALSE(botf::segmentHitsAabb(centerHitbox, kDefaultBox, {-50.f, 25.f}, {50.f, 25.f}));
}

TEST(Hitbox, DegenerateSegmentIsPointTest) {
	const botf::common::Point centerHitbox{10.f, 10.f};
	const botf::common::Point pointHitInside{10.f, 10.f};
	const botf::common::Point pointHitOutside{10.f + kHalf + 1.f, 10.f};
	EXPECT_TRUE(botf::segmentHitsAabb(centerHitbox, kDefaultBox, pointHitInside, pointHitInside));
	EXPECT_FALSE(
		botf::segmentHitsAabb(centerHitbox, kDefaultBox, pointHitOutside, pointHitOutside));
}

TEST(Hitbox, FigureSpawnCarriesDefaultHitbox) {
	botf::GameLogic game;
	const auto id = game.spawnPlayer(botf::common::Point{100.f, 100.f},
									 botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	ASSERT_EQ(game.figures().size(), 1u);
	EXPECT_EQ(game.figures().front().getId(), id);

	const auto& figure = game.figures().front();
	EXPECT_FLOAT_EQ(figure.getHitbox().width, botf::common::GlobalSettings::hitboxSize);
	EXPECT_FLOAT_EQ(figure.getHitbox().height, botf::common::GlobalSettings::hitboxSize);

	const botf::common::Point centerHitbox = figure.getPosition();
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, figure.getHitbox(), {100.f, 100.f}));
	EXPECT_TRUE(botf::pointHitsAabb(centerHitbox, figure.getHitbox(), {100.f + kHalf, 100.f}));
	EXPECT_FALSE(
		botf::pointHitsAabb(centerHitbox, figure.getHitbox(), {100.f + kHalf + 0.01f, 100.f}));
}

TEST(Hitbox, CustomDescUsedInPointTest) {
	const botf::ObjectHitbox wide{80.f, 20.f};
	const botf::common::Point center{0.f, 0.f};
	EXPECT_TRUE(botf::pointHitsAabb(center, wide, {39.f, 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(center, wide, {41.f, 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(center, wide, {0.f, 11.f}));
}
