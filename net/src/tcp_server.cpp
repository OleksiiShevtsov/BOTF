#include "net/tcp_server.hpp"

#include "common/log.hpp"
#include "net/packet_framing.hpp"

#include <array>
#include <utility>

namespace {

void appendBytes(std::vector<std::uint8_t>& dst, const std::vector<std::uint8_t>& src) {
	dst.insert(dst.end(), src.begin(), src.end());
}

}  // namespace

botf::net::TcpServer::TcpServer(InboundQueue& inbound, OutboundQueue& outbound)
	: m_inbound(inbound), m_outbound(outbound) {}

botf::net::TcpServer::~TcpServer() {
	stop();
}

bool botf::net::TcpServer::listen(std::uint16_t port) {
	stop();

	m_listener.setBlocking(false);
	if (m_listener.listen(port) != sf::Socket::Status::Done) {
		botf::log::error("[net] failed to listen on port {}", port);
		return false;
	}

	m_selector.add(m_listener);
	m_port = port;
	m_listening = true;
	botf::log::info("[net] listening on port {}", m_port);
	return true;
}

void botf::net::TcpServer::stop() {
	if (!m_listening && m_clients.empty()) {
		return;
	}

	for (auto& [id, client] : m_clients) {
		(void)id;
		m_selector.remove(*client.socket);
		client.socket->disconnect();
	}
	m_clients.clear();

	m_selector.remove(m_listener);
	m_listener.close();
	m_listening = false;
	m_port = 0;
}

void botf::net::TcpServer::poll() {
	if (!m_listening) {
		return;
	}

	dispatchOutbound();

	if (m_selector.wait(sf::milliseconds(1))) {
		acceptNewClients();
		receiveFromClients();
	}

	flushSendBuffers();
}

void botf::net::TcpServer::acceptNewClients() {
	if (!m_selector.isReady(m_listener)) {
		return;
	}

	auto socket = std::make_unique<sf::TcpSocket>();
	if (m_listener.accept(*socket) != sf::Socket::Status::Done) {
		return;
	}

	socket->setBlocking(false);
	const botf::ClientId id = m_nextClientId++;
	m_selector.add(*socket);

	Client client;
	client.id = id;
	client.socket = std::move(socket);
	m_clients.emplace(id, std::move(client));

	botf::log::info("[net] client connected id={} (total {})", id, m_clients.size());
}

void botf::net::TcpServer::receiveFromClients() {
	std::vector<botf::ClientId> toDrop;

	for (auto& [id, client] : m_clients) {
		if (!m_selector.isReady(*client.socket)) {
			continue;
		}

		std::array<char, 4096> chunk{};
		std::size_t received = 0;
		const auto status = client.socket->receive(chunk.data(), chunk.size(), received);

		if (status == sf::Socket::Status::Disconnected || status == sf::Socket::Status::Error) {
			toDrop.push_back(id);
			continue;
		}
		if (status != sf::Socket::Status::Done || received == 0) {
			continue;
		}

		client.recvBuffer.insert(client.recvBuffer.end(), chunk.data(), chunk.data() + received);

		while (auto payload = tryConsumeFrame(client.recvBuffer)) {
			m_inbound.push(InboundNotification{id, std::move(*payload)});
		}
	}

	for (const botf::ClientId id : toDrop) {
		dropClient(id);
	}
}

void botf::net::TcpServer::dispatchOutbound() {
	while (auto note = m_outbound.tryPop()) {
		const auto framed = framePayload(note->payload);

		if (!note->clientId.has_value()) {
			for (auto& [id, client] : m_clients) {
				(void)id;
				appendBytes(client.sendBuffer, framed);
			}
			continue;
		}

		const auto it = m_clients.find(*note->clientId);
		if (it == m_clients.end()) {
			continue;
		}
		appendBytes(it->second.sendBuffer, framed);
	}
}

void botf::net::TcpServer::flushSendBuffers() {
	std::vector<botf::ClientId> toDrop;

	for (auto& [id, client] : m_clients) {
		while (!client.sendBuffer.empty()) {
			std::size_t sent = 0;
			const auto status =
				client.socket->send(client.sendBuffer.data(), client.sendBuffer.size(), sent);

			if (status == sf::Socket::Status::Disconnected || status == sf::Socket::Status::Error) {
				toDrop.push_back(id);
				break;
			}

			if (sent > 0) {
				client.sendBuffer.erase(
					client.sendBuffer.begin(),
					client.sendBuffer.begin() + static_cast<std::ptrdiff_t>(sent));
			}

			if (status == sf::Socket::Status::Partial || status == sf::Socket::Status::NotReady) {
				break;
			}
			if (status != sf::Socket::Status::Done) {
				break;
			}
		}
	}

	for (const botf::ClientId id : toDrop) {
		dropClient(id);
	}
}

void botf::net::TcpServer::dropClient(botf::ClientId id) {
	const auto it = m_clients.find(id);
	if (it == m_clients.end()) {
		return;
	}

	m_selector.remove(*it->second.socket);
	it->second.socket->disconnect();
	m_clients.erase(it);
	botf::log::info("[net] client disconnected id={} (total {})", id, m_clients.size());
}
