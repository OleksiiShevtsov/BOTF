#include "engine/game_session.h"

#include "common/log.hpp"
#include "utils/random.hpp"

#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

botf::GameSession::GameSession(const common::ID id, std::shared_ptr<const GameMap> gameMap)
	: m_id{id}, m_gameMap{std::move(gameMap)} {
	if (!m_gameMap) {
		throw std::runtime_error("GameSession: gameMap is null");
	}
	if (m_gameMap->getSpawns().empty()) {
		throw std::runtime_error("GameSession: gameMap has no spawns");
	}
}

botf::common::ID botf::GameSession::getId() const {
	return m_id;
}

const botf::GameMap& botf::GameSession::getGameMap() const {
	return *m_gameMap;
}

botf::SessionPhase botf::GameSession::phase() const {
	return m_phase;
}

std::size_t botf::GameSession::seatCapacity() const {
	return m_gameMap->getSpawns().size();
}

bool botf::GameSession::hasFreeSeats(const std::size_t occupied) const {
	return m_phase == SessionPhase::Forming && occupied < seatCapacity();
}

bool botf::GameSession::beginPlaying(const std::vector<common::ID>& clients) {
	if (m_phase != SessionPhase::Forming) {
		botf::log::warn("[server] beginPlaying refused: session {} phase != Forming", m_id);
		return false;
	}
	if (clients.size() != seatCapacity()) {
		botf::log::warn(
			"[server] beginPlaying refused: session {} clients={} capacity={}", m_id,
			clients.size(), seatCapacity());
		return false;
	}
	if (!m_clientSpawnIndex.empty() || !m_gameLogic.figures().empty()) {
		botf::log::error("[server] beginPlaying: session {} already has figures", m_id);
		return false;
	}

	const std::vector<common::Point>& spawns = m_gameMap->getSpawns();
	std::vector<std::size_t> seatOrder(spawns.size());
	std::iota(seatOrder.begin(), seatOrder.end(), 0);
	// Fisher–Yates via shared utils RNG.
	for (std::size_t i = seatOrder.size(); i > 1; --i) {
		const std::size_t j = utils::randomIndex(i);
		std::swap(seatOrder[i - 1], seatOrder[j]);
	}

	for (std::size_t i = 0; i < clients.size(); ++i) {
		const common::ID clientId = clients[i];
		const std::size_t spawnIndex = seatOrder[i];
		m_clientSpawnIndex.emplace(clientId, spawnIndex);
		const common::ID entity = m_gameLogic.spawnPlayer(spawns[spawnIndex]);
		m_gameLogic.bindClient(clientId, entity);
		botf::log::info(
			"[server] session {} seated client {} → entity {} spawn={}", m_id, clientId, entity,
			spawnIndex);
	}

	m_phase = SessionPhase::Playing;
	botf::log::info("[server] session {} Forming → Playing ({} players)", m_id, clients.size());
	return true;
}

void botf::GameSession::removeClient(const common::ID clientId) {
	m_clientSpawnIndex.erase(clientId);
	m_gameLogic.removeClient(clientId);
}

void botf::GameSession::applyClientInput(const common::ID clientId,
										const common::ClientInput& clientInput) {
	if (m_phase != SessionPhase::Playing) {
		return;
	}
	m_gameLogic.setClientInput(clientId, clientInput.dx, clientInput.dy);
	m_gameLogic.setClientAim(clientId, clientInput.aim);
	if (clientInput.fire) {
		m_gameLogic.tryFire(clientId);
	}
}

void botf::GameSession::tick(const float dt) {
	if (m_phase != SessionPhase::Playing) {
		return;
	}
	m_gameLogic.tick(dt);
}

botf::common::WorldSnapshot botf::GameSession::buildWorldSnapshot(const std::uint32_t tick) const {
	common::WorldSnapshot snapshot;
	snapshot.tick = tick;
	snapshot.figures.reserve(m_gameLogic.figures().size());
	for (const auto& figure : m_gameLogic.figures()) {
		snapshot.figures.push_back(common::FigurePose{
			figure.getId(),
			figure.getPosition(),
			figure.getAngle(),
			figure.getHp(),
			figure.getLifeStatus(),
		});
	}
	snapshot.bullets.reserve(m_gameLogic.bullets().size());
	for (const auto& bullet : m_gameLogic.bullets()) {
		snapshot.bullets.push_back(common::BulletPose{
			bullet.getId(),
			bullet.getOwnerId(),
			bullet.getPosition(),
			bullet.getVelocity(),
		});
	}
	return snapshot;
}

const std::vector<botf::FigureObject>& botf::GameSession::figures() const {
	return m_gameLogic.figures();
}

const std::vector<botf::BulletObject>& botf::GameSession::bullets() const {
	return m_gameLogic.bullets();
}

std::optional<botf::common::ID> botf::GameSession::entityForClient(const common::ID clientId) const {
	return m_gameLogic.entityForClient(clientId);
}
