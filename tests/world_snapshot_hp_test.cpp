#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "common/types.hpp"
#include "protocol/protocol.hpp"

#include <gtest/gtest.h>

TEST(WorldSnapshotHp, RoundTripHpAndLifeStatus) {
	botf::common::WorldSnapshot snapshot;
	snapshot.tick = 7;
	snapshot.figures.push_back(botf::common::FigurePose{
		3,
		botf::common::Point{11.f, 22.f},
		0.5f,
		botf::common::GlobalSettings::maxHp - botf::common::GlobalSettings::bulletDamage,
		botf::common::LifeStatus::Alive,
	});
	snapshot.figures.push_back(botf::common::FigurePose{
		4,
		botf::common::Point{0.f, 0.f},
		1.0f,
		0,
		botf::common::LifeStatus::Dead,
	});

	const auto bytes = botf::protocol::encodeWorldSnapshot(snapshot, 1);
	auto decoded = botf::protocol::decode(bytes);
	ASSERT_TRUE(decoded.has_value());
	const auto* world = std::get_if<botf::common::WorldSnapshot>(&decoded->body);
	ASSERT_NE(world, nullptr);
	ASSERT_EQ(world->figures.size(), 2u);

	EXPECT_EQ(world->figures[0].hitPoints,
			  botf::common::GlobalSettings::maxHp - botf::common::GlobalSettings::bulletDamage);
	EXPECT_EQ(world->figures[0].lifeStatus, botf::common::LifeStatus::Alive);
	EXPECT_EQ(world->figures[1].hitPoints, 0);
	EXPECT_EQ(world->figures[1].lifeStatus, botf::common::LifeStatus::Dead);
}
