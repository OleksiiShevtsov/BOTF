#include "app/input_handler.h"
#include "app/match.h"
#include "app/player.h"
#include "common/dto.hpp"
#include "net/packet_framing.hpp"
#include "protocol/protocol.hpp"

#include <variant>
#include <vector>

#include <gtest/gtest.h>

TEST(ClientSmoke, WorldSnapshotAppliesToMatch) {
	botf::app::InputHandler input;
	const auto sample = input.readMoveInput();
	const auto movePacket = botf::net::framePayload(
		botf::protocol::encodeMove(botf::common::MoveCommand{sample.dx, sample.dy, 1}, 1));
	EXPECT_FALSE(movePacket.empty());

	botf::common::WorldSnapshot snapshot;
	snapshot.tick = 42;
	snapshot.players.push_back(botf::common::PlayerPose{
		7,
		botf::common::Point{100.f, 300.f},
		1.25f,
	});

	const auto framed =
		botf::net::framePayload(botf::protocol::encodeWorldSnapshot(snapshot, /*sequence=*/1));

	std::vector<std::uint8_t> buffer(framed.begin(), framed.end());
	auto consumed = botf::net::tryConsumeFrame(buffer);
	ASSERT_TRUE(consumed.has_value());

	auto decoded = botf::protocol::decode(*consumed);
	ASSERT_TRUE(decoded.has_value());
	const auto* world = std::get_if<botf::protocol::DecodedWorldState>(&*decoded);
	ASSERT_NE(world, nullptr);
	ASSERT_EQ(world->snapshot.players.size(), 1u);

	botf::app::Match match;
	match.applyWorldSnapshot(world->snapshot);
	ASSERT_EQ(match.getScene().size(), 1u);

	const botf::app::IViewObject* view = match.getScene().find(7);
	ASSERT_NE(view, nullptr);
	EXPECT_EQ(view->getId(), 7u);
	EXPECT_FLOAT_EQ(view->getPosition().x, 100.f);
	EXPECT_FLOAT_EQ(view->getPosition().y, 300.f);
	EXPECT_FLOAT_EQ(view->getAngle(), 1.25f);

	// Upsert keeps the same object and updates pose.
	botf::common::WorldSnapshot next = world->snapshot;
	next.players[0].position = botf::common::Point{110.f, 310.f};
	match.applyWorldSnapshot(next);
	ASSERT_EQ(match.getScene().size(), 1u);
	view = match.getScene().find(7);
	ASSERT_NE(view, nullptr);
	EXPECT_FLOAT_EQ(view->getPosition().x, 110.f);
	EXPECT_FLOAT_EQ(view->getPosition().y, 310.f);

	botf::app::Player localPlayer;
	localPlayer.setId(1);
	EXPECT_EQ(localPlayer.getId(), 1u);

	EXPECT_EQ(world->snapshot.tick, 42u);
	EXPECT_FALSE(framed.empty());
}
