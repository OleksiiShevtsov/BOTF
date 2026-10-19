#include "engine/server_engine.h"
#include "helpers/test_env.h"
#include "net/packet_framing.hpp"
#include "protocol/protocol.hpp"

#include <array>
#include <chrono>
#include <optional>
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

std::optional<botf::common::ObjectID> helloBind(botf::ServerEngine& engine,
												sf::TcpSocket& client,
												std::vector<std::uint8_t>& recvBuffer,
												botf::common::ClientID expectedClientId,
												std::uint64_t sequence) {
	EXPECT_TRUE(
		sendFramed(client, botf::protocol::encodeHello(botf::common::HelloMessage{}, sequence)));

	for (int i = 0; i < 200; ++i) {
		engine.pollNetwork();
		engine.applyInbound();
		engine.pollNetwork();

		std::string payload;
		if (!recvOneFrame(client, recvBuffer, payload, 1)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}

		auto decoded = botf::protocol::decode(payload);
		if (!decoded) {
			continue;
		}
		const auto* hello = std::get_if<botf::common::HelloMessage>(&decoded->body);
		if (!hello || hello->objectId == 0) {
			continue;
		}

		EXPECT_EQ(engine.game().entityForClient(expectedClientId), hello->objectId);
		return hello->objectId;
	}
	return std::nullopt;
}

const botf::common::PlayerPose* findPlayer(const botf::common::WorldSnapshot& snapshot,
										   botf::common::ObjectID id) {
	for (const auto& player : snapshot.players) {
		if (player.id == id) {
			return &player;
		}
	}
	return nullptr;
}

botf::common::ClientInput makeInput(int dx,
									int dy,
									std::uint32_t tick,
									botf::common::Point aim = {200.f, 100.f}) {
	return botf::common::ClientInput{dx, dy, aim, false, tick};
}

}  // namespace

TEST(ServerSmoke, HelloBindThenClientInputWorldState) {
	const auto port = botf::test::ephemeralPort();
	ASSERT_TRUE(port.has_value());

	botf::ServerEngine engine;
	ASSERT_TRUE(engine.startListen(*port));
	EXPECT_TRUE(engine.game().objects().empty());

	sf::TcpSocket client;
	client.setBlocking(true);
	ASSERT_EQ(client.connect(sf::IpAddress::LocalHost, *port, sf::seconds(2)),
			  sf::Socket::Status::Done);

	std::vector<std::uint8_t> recvBuffer;
	client.setBlocking(false);
	const auto selfId = helloBind(engine, client, recvBuffer, 1, 1);
	ASSERT_TRUE(selfId.has_value());
	ASSERT_EQ(engine.game().objects().size(), 1u);
	ASSERT_EQ(engine.game().objects().front().id(), *selfId);

	const auto start = engine.game().objects().front().position();

	struct Sample {
		int dx;
		int dy;
		std::uint32_t tick;
		std::uint64_t seq;
	};
	const Sample samples[] = {
		{0, 1, 1, 2},
		{1, 1, 2, 3},
		{-1, 0, 3, 4},
		{0, 0, 4, 5},
	};

	client.setBlocking(true);
	for (const auto& sample : samples) {
		ASSERT_TRUE(
			sendFramed(client, botf::protocol::encodeClientInput(
								   makeInput(sample.dx, sample.dy, sample.tick), sample.seq)));

		bool got = false;
		for (int i = 0; i < 100 && !got; ++i) {
			engine.pollNetwork();
			engine.applyInbound();
			const auto input = engine.lastClientInput(1);
			if (input && input->tick == sample.tick) {
				got = true;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
		ASSERT_TRUE(got) << "waiting for ClientInput tick=" << sample.tick;

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
		const auto* world = std::get_if<botf::common::WorldSnapshot>(&decoded->body);
		if (!world || world->players.empty()) {
			continue;
		}
		lastSnapshot = *world;
		gotWorld = true;
	}

	client.disconnect();

	const auto last = engine.lastClientInput(1);
	ASSERT_TRUE(last.has_value());
	EXPECT_EQ(last->tick, 4u);
	EXPECT_TRUE(botf::protocol::isIdle(*last));

	const auto end = engine.game().objects().front().position();
	EXPECT_FALSE(end.x == start.x && end.y == start.y) << "DynamicObject did not move";

	ASSERT_TRUE(gotWorld);
	const auto* pose = findPlayer(lastSnapshot, *selfId);
	ASSERT_NE(pose, nullptr);
	const float dx = pose->position.x - end.x;
	const float dy = pose->position.y - end.y;
	EXPECT_LE(dx * dx + dy * dy, 0.01f);

	engine.shutdown();
}

TEST(ServerSmoke, TwoClientsHelloOwnEntityAndInputIsolation) {
	const auto port = botf::test::ephemeralPort();
	ASSERT_TRUE(port.has_value());

	botf::ServerEngine engine;
	ASSERT_TRUE(engine.startListen(*port));

	sf::TcpSocket clientA;
	sf::TcpSocket clientB;
	clientA.setBlocking(true);
	clientB.setBlocking(true);
	ASSERT_EQ(clientA.connect(sf::IpAddress::LocalHost, *port, sf::seconds(2)),
			  sf::Socket::Status::Done);
	ASSERT_EQ(clientB.connect(sf::IpAddress::LocalHost, *port, sf::seconds(2)),
			  sf::Socket::Status::Done);

	std::vector<std::uint8_t> bufferA;
	std::vector<std::uint8_t> bufferB;
	clientA.setBlocking(false);
	clientB.setBlocking(false);

	const auto idA = helloBind(engine, clientA, bufferA, 1, 1);
	const auto idB = helloBind(engine, clientB, bufferB, 2, 1);
	ASSERT_TRUE(idA.has_value());
	ASSERT_TRUE(idB.has_value());
	EXPECT_NE(*idA, *idB);
	ASSERT_EQ(engine.game().objects().size(), 2u);

	botf::common::Point startA{};
	botf::common::Point startB{};
	for (const auto& obj : engine.game().objects()) {
		if (obj.id() == *idA) {
			startA = obj.position();
		}
		if (obj.id() == *idB) {
			startB = obj.position();
		}
	}

	clientA.setBlocking(true);
	ASSERT_TRUE(sendFramed(clientA, botf::protocol::encodeClientInput(makeInput(1, 0, 10), 2)));

	bool gotInput = false;
	for (int i = 0; i < 100 && !gotInput; ++i) {
		engine.pollNetwork();
		engine.applyInbound();
		const auto input = engine.lastClientInput(1);
		if (input && input->tick == 10) {
			gotInput = true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	ASSERT_TRUE(gotInput);

	for (int j = 0; j < 20; ++j) {
		engine.advanceSim(kDt);
		engine.pollNetwork();
	}

	botf::common::Point endA{};
	botf::common::Point endB{};
	for (const auto& obj : engine.game().objects()) {
		if (obj.id() == *idA) {
			endA = obj.position();
		}
		if (obj.id() == *idB) {
			endB = obj.position();
		}
	}

	EXPECT_FALSE(endA.x == startA.x && endA.y == startA.y) << "client A entity should move";
	EXPECT_FLOAT_EQ(endB.x, startB.x);
	EXPECT_FLOAT_EQ(endB.y, startB.y);

	// Unbound ClientInput (no Hello) must not spawn a third entity.
	sf::TcpSocket clientC;
	clientC.setBlocking(true);
	ASSERT_EQ(clientC.connect(sf::IpAddress::LocalHost, *port, sf::seconds(2)),
			  sf::Socket::Status::Done);
	ASSERT_TRUE(sendFramed(clientC, botf::protocol::encodeClientInput(makeInput(0, 1, 1), 1)));
	for (int i = 0; i < 50; ++i) {
		engine.pollNetwork();
		engine.applyInbound();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	EXPECT_EQ(engine.game().objects().size(), 2u);
	EXPECT_FALSE(engine.game().entityForClient(3).has_value());

	clientA.disconnect();
	clientB.disconnect();
	clientC.disconnect();
	engine.shutdown();
}

TEST(ServerSmoke, HelloClientInputAimUpdatesEntityAngleInWorldState) {
	const auto port = botf::test::ephemeralPort();
	ASSERT_TRUE(port.has_value());

	botf::ServerEngine engine;
	ASSERT_TRUE(engine.startListen(*port));

	sf::TcpSocket client;
	client.setBlocking(true);
	ASSERT_EQ(client.connect(sf::IpAddress::LocalHost, *port, sf::seconds(2)),
			  sf::Socket::Status::Done);

	std::vector<std::uint8_t> recvBuffer;
	client.setBlocking(false);
	const auto selfId = helloBind(engine, client, recvBuffer, 1, 1);
	ASSERT_TRUE(selfId.has_value());

	const auto pos = engine.game().objects().front().position();
	// Aim due +X from entity → expected angle 0.
	const botf::common::Point aimPoint{pos.x + 100.f, pos.y};
	client.setBlocking(true);
	ASSERT_TRUE(
		sendFramed(client, botf::protocol::encodeClientInput(makeInput(0, 0, 7, aimPoint), 2)));

	bool gotInput = false;
	for (int i = 0; i < 100 && !gotInput; ++i) {
		engine.pollNetwork();
		engine.applyInbound();
		const auto input = engine.lastClientInput(1);
		if (input && input->tick == 7) {
			gotInput = true;
			EXPECT_FLOAT_EQ(input->aim.x, aimPoint.x);
			EXPECT_FLOAT_EQ(input->aim.y, aimPoint.y);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	ASSERT_TRUE(gotInput);

	EXPECT_NEAR(engine.game().objects().front().angle(), 0.f, 1e-4f);

	engine.advanceSim(kDt);
	engine.pollNetwork();

	client.setBlocking(false);
	bool gotWorld = false;
	float worldAngle = 0.f;
	for (int i = 0; i < 200 && !gotWorld; ++i) {
		engine.pollNetwork();
		std::string payload;
		if (!recvOneFrame(client, recvBuffer, payload, 1)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}
		auto decoded = botf::protocol::decode(payload);
		if (!decoded) {
			continue;
		}
		const auto* world = std::get_if<botf::common::WorldSnapshot>(&decoded->body);
		if (!world) {
			continue;
		}
		const auto* pose = findPlayer(*world, *selfId);
		if (!pose) {
			continue;
		}
		worldAngle = pose->angle;
		gotWorld = true;
	}

	ASSERT_TRUE(gotWorld);
	EXPECT_NEAR(worldAngle, 0.f, 1e-4f);

	client.disconnect();
	engine.shutdown();
}
