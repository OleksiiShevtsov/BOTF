#include "common/global_settings.hpp"
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
	const botf::common::Point center{100.f, 200.f};

	EXPECT_TRUE(botf::pointHitsAabb(center, center));
	EXPECT_TRUE(botf::pointHitsAabb(center, {100.f + kHalf, 200.f}));
	EXPECT_TRUE(botf::pointHitsAabb(center, {100.f - kHalf, 200.f}));
	EXPECT_TRUE(botf::pointHitsAabb(center, {100.f, 200.f + kHalf}));
	EXPECT_TRUE(botf::pointHitsAabb(center, {100.f, 200.f - kHalf}));
	// Triangle vertices from card 16 sit on the AABB edge.
	EXPECT_TRUE(botf::pointHitsAabb(center, {100.f, 200.f + kHalf}));		  // (0,+20)
	EXPECT_TRUE(botf::pointHitsAabb(center, {100.f - kHalf, 200.f - kHalf})); // (-20,-20)
	EXPECT_TRUE(botf::pointHitsAabb(center, {100.f + kHalf, 200.f - kHalf})); // (20,-20)
}

TEST(Hitbox, PointJustOutsideIsMiss) {
	const botf::common::Point center{0.f, 0.f};
	constexpr float kEps = 0.01f;

	EXPECT_FALSE(botf::pointHitsAabb(center, {kHalf + kEps, 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(center, {-(kHalf + kEps), 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(center, {0.f, kHalf + kEps}));
	EXPECT_FALSE(botf::pointHitsAabb(center, {0.f, -(kHalf + kEps)}));
	EXPECT_FALSE(botf::pointHitsAabb(center, {kHalf + kEps, kHalf + kEps}));
}

TEST(Hitbox, SegmentCrossingAabbIsHit) {
	const botf::common::Point center{0.f, 0.f};
	// Passes through the square horizontally.
	EXPECT_TRUE(botf::segmentHitsAabb(center, {-50.f, 0.f}, {50.f, 0.f}));
	// Starts outside, ends inside.
	EXPECT_TRUE(botf::segmentHitsAabb(center, {-50.f, 0.f}, {0.f, 0.f}));
	// Touches a corner.
	EXPECT_TRUE(botf::segmentHitsAabb(center, {kHalf, kHalf}, {kHalf + 10.f, kHalf + 10.f}));
}

TEST(Hitbox, SegmentMissingAabbIsMiss) {
	const botf::common::Point center{0.f, 0.f};
	EXPECT_FALSE(botf::segmentHitsAabb(center, {30.f, 30.f}, {40.f, 40.f}));
	EXPECT_FALSE(botf::segmentHitsAabb(center, {-50.f, 25.f}, {50.f, 25.f}));
}

TEST(Hitbox, DegenerateSegmentIsPointTest) {
	const botf::common::Point center{10.f, 10.f};
	const botf::common::Point inside{10.f, 10.f};
	const botf::common::Point outside{10.f + kHalf + 1.f, 10.f};
	EXPECT_TRUE(botf::segmentHitsAabb(center, inside, inside));
	EXPECT_FALSE(botf::segmentHitsAabb(center, outside, outside));
}
