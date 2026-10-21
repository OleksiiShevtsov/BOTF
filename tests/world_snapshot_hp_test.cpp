#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "common/life_status.hpp"
#include "protocol/protocol.hpp"

#include <gtest/gtest.h>

TEST(WorldSnapshotHp, RoundTripHpAndLifeStatus) {
	botf::common::WorldSnapshot snapshot;
	snapshot.tick = 7;
	snapshot.players.push_back(botf::common::PlayerPose{
		3,
		botf::common::Point{11.f, 22.f},
		0.5f,
		botf::common::GlobalSettings::maxHp - botf::common::GlobalSettings::bulletDamage,
		botf::common::LifeStatus::Alive,
	});
	snapshot.players.push_back(botf::common::PlayerPose{
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
	ASSERT_EQ(world->players.size(), 2u);

	EXPECT_EQ(world->players[0].hp,
			  botf::common::GlobalSettings::maxHp - botf::common::GlobalSettings::bulletDamage);
	EXPECT_EQ(world->players[0].lifeStatus, botf::common::LifeStatus::Alive);
	EXPECT_EQ(world->players[1].hp, 0);
	EXPECT_EQ(world->players[1].lifeStatus, botf::common::LifeStatus::Dead);
}
