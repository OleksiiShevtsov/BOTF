#include "engine/server_engine.h"
#include "helpers/test_env.h"
#include "net/packet_framing.hpp"
#include "protocol/protocol.hpp"

#include <array>
#include <chrono>
#include <thread>
#include <variant>
#include <vector>

#include <gtest/gtest.h>
#include <SFML/Network.hpp>

namespace {

constexpr float kDt = 1.0f / 20.0f;

bool sendFramed(sf::TcpSocket& socket, const std::string& envelopeBytes) {
	const auto packet = botf::net::framePayload(envelopeBytes);
	return socket.send(packet.data(), packet.size()) == sf::Socket::Status::Done;
}

bool recvOneFrame(sf::TcpSocket& socket,
				  std::vector<std::uint8_t>& buffer,
				  std::string& outPayload,
				  int attempts) {
	for (int i = 0; i < attempts; ++i) {
		if (auto payload = botf::net::tryConsumeFrame(buffer)) {
			outPayload = std::move(*payload);
			return true;
		}

		std::array<char, 4096> chunk{};
		std::size_t received = 0;
		const auto status = socket.receive(chunk.data(), chunk.size(), received);
		if (status == sf::Socket::Status::Done && received > 0) {
			buffer.insert(buffer.end(), chunk.data(), chunk.data() + received);
			continue;
		}
		if (status == sf::Socket::Status::Disconnected || status == sf::Socket::Status::Error) {
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	return false;
}

}  // namespace

TEST(ServerSmoke, MoveTickWorldStateRoundTrip) {
	const auto port = botf::test::ephemeralPort();
	ASSERT_TRUE(port.has_value());

	botf::ServerEngine engine;
	ASSERT_TRUE(engine.startListen(*port));

	sf::TcpSocket client;
	client.setBlocking(true);
	ASSERT_EQ(client.connect(sf::IpAddress::LocalHost, *port, sf::seconds(2)),
			  sf::Socket::Status::Done);

	struct Sample {
		int dx;
		int dy;
		std::uint32_t tick;
		std::uint64_t seq;
	};
	const Sample samples[] = {
		{0, 1, 1, 1},
		{1, 1, 2, 2},
		{-1, 0, 3, 3},
		{0, 0, 4, 4},
	};

	ASSERT_FALSE(engine.game().objects().empty());
	const auto start = engine.game().objects().front().position();
	std::vector<std::uint8_t> recvBuffer;

	for (const auto& sample : samples) {
		ASSERT_TRUE(sendFramed(
			client, botf::protocol::encodeMove(
						botf::common::MoveCommand{sample.dx, sample.dy, sample.tick}, sample.seq)));

		bool got = false;
		for (int i = 0; i < 100 && !got; ++i) {
			engine.pollNetwork();
			engine.applyInboundMoves();
			const auto move = engine.lastMove(1);
			if (move && move->tick == sample.tick) {
				got = true;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
		ASSERT_TRUE(got) << "waiting for Move tick=" << sample.tick;

		for (int j = 0; j < 15; ++j) {
			engine.advanceSim(kDt);
			engine.pollNetwork();
		}
	}

	for (int i = 0; i < 20; ++i) {
		engine.pollNetwork();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	client.setBlocking(false);

	botf::common::WorldSnapshot lastSnapshot;
	bool gotWorld = false;
	for (int i = 0; i < 500; ++i) {
		engine.pollNetwork();
		std::string payload;
		if (!recvOneFrame(client, recvBuffer, payload, 1)) {
			if (gotWorld && i > 50) {
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}
		auto decoded = botf::protocol::decode(payload);
		if (!decoded) {
			continue;
		}
		const auto* world = std::get_if<botf::protocol::DecodedWorldState>(&*decoded);
		if (!world || world->snapshot.players.empty()) {
			continue;
		}
		lastSnapshot = world->snapshot;
		gotWorld = true;
	}

	client.disconnect();

	const auto last = engine.lastMove(1);
	ASSERT_TRUE(last.has_value());
	EXPECT_EQ(last->tick, 4u);
	EXPECT_TRUE(botf::protocol::isIdle(*last));

	const auto end = engine.game().objects().front().position();
	EXPECT_FALSE(end.x == start.x && end.y == start.y) << "DynamicObject did not move";

	ASSERT_TRUE(gotWorld);
	const auto& p0 = lastSnapshot.players.front();
	const float dx = p0.position.x - end.x;
	const float dy = p0.position.y - end.y;
	EXPECT_LE(dx * dx + dy * dy, 0.01f);

	engine.shutdown();
}
