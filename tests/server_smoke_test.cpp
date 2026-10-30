#include "engine/game_session.h"
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

bool sendJoinSession(sf::TcpSocket& client, std::uint64_t sequence) {
	return sendFramed(client,
					  botf::protocol::encodeJoinSession(botf::common::JoinSessionMessage{}, sequence));
}

// Pump engine until this socket receives SessionStart; returns bound entity id.
std::optional<botf::common::ID> awaitSessionStart(botf::ServerEngine& engine,
												  sf::TcpSocket& client,
												  std::vector<std::uint8_t>& recvBuffer) {
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
		const auto* start = std::get_if<botf::common::SessionStartMessage>(&decoded->body);
		if (!start || start->boundId == 0) {
			continue;
		}
		return start->boundId;
	}
	return std::nullopt;
}

// Map has 2 seats: two JoinSessions fill Forming → Playing + SessionStart to both.
struct TwoPlayerMatch {
	botf::common::ID entityA{};
	botf::common::ID entityB{};
};

std::optional<TwoPlayerMatch> startTwoPlayerMatch(botf::ServerEngine& engine,
												  sf::TcpSocket& clientA,
												  sf::TcpSocket& clientB,
												  std::vector<std::uint8_t>& bufferA,
												  std::vector<std::uint8_t>& bufferB) {
	if (!engine.getGameSessionsPool().empty()) {
		return std::nullopt;
	}

	clientA.setBlocking(false);
	clientB.setBlocking(false);

	if (!sendJoinSession(clientA, 1)) {
		return std::nullopt;
	}
	for (int i = 0; i < 50; ++i) {
		engine.pollNetwork();
		engine.applyInbound();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	botf::GameSession* forming = engine.getGameSessionsPool().findGameSessionByClientId(1);
	if (engine.getGameSessionsPool().size() != 1u || !forming ||
		forming->phase() != botf::SessionPhase::Forming || !forming->figures().empty()) {
		return std::nullopt;
	}

	if (!sendJoinSession(clientB, 1)) {
		return std::nullopt;
	}

	const auto idA = awaitSessionStart(engine, clientA, bufferA);
	const auto idB = awaitSessionStart(engine, clientB, bufferB);
	if (!idA || !idB || *idA == *idB) {
		return std::nullopt;
	}

	botf::GameSession* playing = engine.getGameSessionsPool().findGameSessionByClientId(1);
	if (engine.getGameSessionsPool().size() != 1u || !playing ||
		playing->phase() != botf::SessionPhase::Playing ||
		playing != engine.getGameSessionsPool().findGameSessionByClientId(2) ||
		playing->figures().size() != 2u) {
		return std::nullopt;
	}

	return TwoPlayerMatch{*idA, *idB};
}

const botf::common::FigurePose* findFigure(const botf::common::WorldSnapshot& snapshot,
										   botf::common::ID id) {
	for (const auto& figure : snapshot.figures) {
		if (figure.id == id) {
			return &figure;
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

TEST(ServerSmoke, JoinSessionFullStartsPlayingThenClientInputWorldState) {
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
	const auto match = startTwoPlayerMatch(engine, clientA, clientB, bufferA, bufferB);
	ASSERT_TRUE(match.has_value());

	botf::common::Point startA{};
	for (const auto& obj : engine.getGameSessionsPool().findGameSessionByClientId(1)->figures()) {
		if (obj.getId() == match->entityA) {
			startA = obj.getPosition();
		}
	}

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

	clientA.setBlocking(true);
	for (const auto& sample : samples) {
		ASSERT_TRUE(
			sendFramed(clientA, botf::protocol::encodeClientInput(
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

	clientA.setBlocking(false);

	botf::common::WorldSnapshot lastSnapshot;
	bool gotWorld = false;
	for (int i = 0; i < 500; ++i) {
		engine.pollNetwork();
		std::string payload;
		if (!recvOneFrame(clientA, bufferA, payload, 1)) {
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
		if (!world || world->figures.empty()) {
			continue;
		}
		lastSnapshot = *world;
		gotWorld = true;
	}

	const auto last = engine.lastClientInput(1);
	ASSERT_TRUE(last.has_value());
	EXPECT_EQ(last->tick, 4u);
	EXPECT_TRUE(botf::protocol::isIdle(*last));

	botf::common::Point endA{};
	for (const auto& obj : engine.getGameSessionsPool().findGameSessionByClientId(1)->figures()) {
		if (obj.getId() == match->entityA) {
			endA = obj.getPosition();
		}
	}
	EXPECT_FALSE(endA.x == startA.x && endA.y == startA.y) << "FigureObject did not move";

	ASSERT_TRUE(gotWorld);
	const auto* pose = findFigure(lastSnapshot, match->entityA);
	ASSERT_NE(pose, nullptr);
	const float dx = pose->position.x - endA.x;
	const float dy = pose->position.y - endA.y;
	EXPECT_LE(dx * dx + dy * dy, 0.01f);

	clientA.disconnect();
	clientB.disconnect();
	engine.shutdown();
}

TEST(ServerSmoke, TwoClientsInputIsolationAndThirdOpensNewForming) {
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
	const auto match = startTwoPlayerMatch(engine, clientA, clientB, bufferA, bufferB);
	ASSERT_TRUE(match.has_value());

	botf::common::Point startA{};
	botf::common::Point startB{};
	for (const auto& obj : engine.getGameSessionsPool().findGameSessionByClientId(1)->figures()) {
		if (obj.getId() == match->entityA) {
			startA = obj.getPosition();
		}
		if (obj.getId() == match->entityB) {
			startB = obj.getPosition();
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
	for (const auto& obj : engine.getGameSessionsPool().findGameSessionByClientId(1)->figures()) {
		if (obj.getId() == match->entityA) {
			endA = obj.getPosition();
		}
		if (obj.getId() == match->entityB) {
			endB = obj.getPosition();
		}
	}

	EXPECT_FALSE(endA.x == startA.x && endA.y == startA.y) << "client A entity should move";
	EXPECT_FLOAT_EQ(endB.x, startB.x);
	EXPECT_FLOAT_EQ(endB.y, startB.y);

	// Unbound ClientInput (no JoinSession) must not create a session/entity.
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
	EXPECT_EQ(engine.getGameSessionsPool().findGameSessionByClientId(1)->figures().size(), 2u);
	EXPECT_EQ(engine.getGameSessionsPool().findGameSessionByClientId(3), nullptr);

	// Third JoinSession opens a new Forming session (Playing is full).
	clientC.setBlocking(false);
	std::vector<std::uint8_t> bufferC;
	ASSERT_TRUE(sendJoinSession(clientC, 1));
	for (int i = 0; i < 50; ++i) {
		engine.pollNetwork();
		engine.applyInbound();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	ASSERT_EQ(engine.getGameSessionsPool().size(), 2u);
	ASSERT_NE(engine.getGameSessionsPool().findGameSessionByClientId(3), nullptr);
	EXPECT_EQ(engine.getGameSessionsPool().findGameSessionByClientId(3)->phase(),
			  botf::SessionPhase::Forming);
	EXPECT_NE(engine.getGameSessionsPool().findGameSessionByClientId(3),
			  engine.getGameSessionsPool().findGameSessionByClientId(1));

	clientA.disconnect();
	clientB.disconnect();
	clientC.disconnect();
	engine.shutdown();
}

TEST(ServerSmoke, FormingIgnoresClientInputUntilPlaying) {
	const auto port = botf::test::ephemeralPort();
	ASSERT_TRUE(port.has_value());

	botf::ServerEngine engine;
	ASSERT_TRUE(engine.startListen(*port));

	sf::TcpSocket clientA;
	clientA.setBlocking(true);
	ASSERT_EQ(clientA.connect(sf::IpAddress::LocalHost, *port, sf::seconds(2)),
			  sf::Socket::Status::Done);

	ASSERT_TRUE(sendJoinSession(clientA, 1));
	for (int i = 0; i < 50; ++i) {
		engine.pollNetwork();
		engine.applyInbound();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	ASSERT_EQ(engine.getGameSessionsPool().findGameSessionByClientId(1)->phase(),
			  botf::SessionPhase::Forming);

	ASSERT_TRUE(sendFramed(clientA, botf::protocol::encodeClientInput(makeInput(1, 0, 1), 2)));
	for (int i = 0; i < 50; ++i) {
		engine.pollNetwork();
		engine.applyInbound();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
	EXPECT_FALSE(engine.lastClientInput(1).has_value());
	EXPECT_TRUE(engine.getGameSessionsPool().findGameSessionByClientId(1)->figures().empty());

	clientA.disconnect();
	engine.shutdown();
}

TEST(ServerSmoke, PlayingAimUpdatesEntityAngleInWorldState) {
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
	const auto match = startTwoPlayerMatch(engine, clientA, clientB, bufferA, bufferB);
	ASSERT_TRUE(match.has_value());

	botf::common::Point pos{};
	for (const auto& obj : engine.getGameSessionsPool().findGameSessionByClientId(1)->figures()) {
		if (obj.getId() == match->entityA) {
			pos = obj.getPosition();
		}
	}
	const botf::common::Point aimPoint{pos.x + 100.f, pos.y};
	clientA.setBlocking(true);
	ASSERT_TRUE(
		sendFramed(clientA, botf::protocol::encodeClientInput(makeInput(0, 0, 7, aimPoint), 2)));

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

	float angle = 0.f;
	for (const auto& obj : engine.getGameSessionsPool().findGameSessionByClientId(1)->figures()) {
		if (obj.getId() == match->entityA) {
			angle = obj.getAngle();
		}
	}
	EXPECT_NEAR(angle, 0.f, 1e-4f);

	engine.advanceSim(kDt);
	engine.pollNetwork();

	clientA.setBlocking(false);
	bool gotWorld = false;
	float worldAngle = 0.f;
	for (int i = 0; i < 200 && !gotWorld; ++i) {
		engine.pollNetwork();
		std::string payload;
		if (!recvOneFrame(clientA, bufferA, payload, 1)) {
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
		const auto* pose = findFigure(*world, match->entityA);
		if (!pose) {
			continue;
		}
		worldAngle = pose->aimAngle;
		gotWorld = true;
	}

	ASSERT_TRUE(gotWorld);
	EXPECT_NEAR(worldAngle, 0.f, 1e-4f);

	clientA.disconnect();
	clientB.disconnect();
	engine.shutdown();
}
