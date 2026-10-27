#pragma once

#include "net/queues.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include <SFML/Network.hpp>

namespace botf::net {

// SFML TCP server: accepts clients, fills inbound queue, drains outbound queue.
class TcpServer {
public:
	TcpServer(InboundQueue& inbound, OutboundQueue& outbound);
	~TcpServer();

	TcpServer(const TcpServer&) = delete;
	TcpServer& operator=(const TcpServer&) = delete;

	// Bind and listen. Returns false on failure.
	bool listen(std::uint16_t port);

	void stop();

	// Accept / recv / send once. Call from the server loop.
	void poll();

	[[nodiscard]] bool isListening() const { return m_listening; }
	[[nodiscard]] std::size_t clientCount() const { return m_clients.size(); }
	[[nodiscard]] std::uint16_t port() const { return m_port; }

	// Client ids dropped since the last call (clears the buffer).
	[[nodiscard]] std::vector<botf::common::ID> takeDisconnected();

private:
	struct Client {
		botf::common::ID id{};
		std::unique_ptr<sf::TcpSocket> socket;
		std::vector<std::uint8_t> recvBuffer;
		std::vector<std::uint8_t> sendBuffer;
	};

	void acceptNewClients();
	void receiveFromClients();
	void dispatchOutbound();
	void flushSendBuffers();
	void dropClient(botf::common::ID id);

	InboundQueue& m_inbound;
	OutboundQueue& m_outbound;

	sf::TcpListener m_listener;
	sf::SocketSelector m_selector;
	std::unordered_map<botf::common::ID, Client> m_clients;

	botf::common::ID m_nextClientId = 1;
	std::uint16_t m_port = 0;
	bool m_listening = false;
	std::vector<botf::common::ID> m_disconnected;
};

}  // namespace botf::net
