#include "engine/game_session.h"

#include "common/log.hpp"
#include "utils/random.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

botf::GameSession::GameSession(const common::ID id, std::shared_ptr<const GameMap> gameMap)
	: m_id{id}, m_gameMap{std::move(gameMap)} {
	if (!m_gameMap) {
		throw std::runtime_error("GameSession: gameMap is null");
	}
	const std::size_t capacity = m_gameMap->getSpawns().size();
	if (capacity == 0) {
		throw std::runtime_error("GameSession: gameMap has no spawns");
	}
	// Seat index == figure entity id == spawn slot (0..capacity-1).
	m_seats.reserve(capacity);
	for (std::size_t i = 0; i < capacity; ++i) {
		m_seats.emplace_back(i);
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
	return m_seats.size();
}

std::size_t botf::GameSession::clientCount() const {
	std::size_t count = 0;
	for (const Seat& seat : m_seats) {
		if (!seat.isFree()) {
			++count;
		}
	}
	return count;
}

std::vector<botf::common::ID> botf::GameSession::clients() const {
	std::vector<common::ID> ids;
	ids.reserve(m_seats.size());
	for (const Seat& seat : m_seats) {
		if (const std::shared_ptr<Player> player = seat.getPlayer()) {
			ids.push_back(player->getId());
		}
	}
	return ids;
}

const std::vector<botf::Seat>& botf::GameSession::seats() const {
	return m_seats;
}

bool botf::GameSession::hasFreeSeats() const {
	if (m_phase != SessionPhase::Forming) {
		return false;
	}
	for (const Seat& seat : m_seats) {
		if (seat.isFree()) {
			return true;
		}
	}
	return false;
}

botf::Seat* botf::GameSession::findFreeSeat() {
	for (Seat& seat : m_seats) {
		if (seat.isFree()) {
			return &seat;
		}
	}
	return nullptr;
}

botf::Seat* botf::GameSession::findSeatByPlayerId(const common::ID clientId) {
	for (Seat& seat : m_seats) {
		const std::shared_ptr<Player> player = seat.getPlayer();
		if (player && player->getId() == clientId) {
			return &seat;
		}
	}
	return nullptr;
}

const botf::Seat* botf::GameSession::findSeatByPlayerId(const common::ID clientId) const {
	for (const Seat& seat : m_seats) {
		const std::shared_ptr<Player> player = seat.getPlayer();
		if (player && player->getId() == clientId) {
			return &seat;
		}
	}
	return nullptr;
}

bool botf::GameSession::addPlayer(std::shared_ptr<Player> player) {
	if (!player) {
		return false;
	}
	if (findSeatByPlayerId(player->getId()) != nullptr) {
		return true;
	}
	if (m_phase != SessionPhase::Forming) {
		return false;
	}
	Seat* seat = findFreeSeat();
	if (!seat) {
		return false;
	}
	seat->setPlayer(std::move(player));
	return true;
}

bool botf::GameSession::beginPlaying() {
	if (m_phase != SessionPhase::Forming) {
		botf::log::warn("[server] beginPlaying refused: session {} phase != Forming", m_id);
		return false;
	}
	if (clientCount() != seatCapacity()) {
		botf::log::warn(
			"[server] beginPlaying refused: session {} clients={} capacity={}", m_id, clientCount(),
			seatCapacity());
		return false;
	}
	if (!m_gameLogic.figures().empty()) {
		botf::log::error("[server] beginPlaying: session {} already has figures", m_id);
		return false;
	}

	randomShuffleIndex();

	const std::vector<common::Point>& spawns = m_gameMap->getSpawns();
	for (Seat& seat : m_seats) {
		const std::shared_ptr<Player> player = seat.getPlayer();
		const common::ID entityId = static_cast<common::ID>(seat.getIndex());
		if (!m_gameLogic.spawnFigure(entityId, spawns[seat.getIndex()])) {
			botf::log::error(
				"[server] beginPlaying: session {} failed to spawn figure id {}", m_id, entityId);
			for (Seat& spawned : m_seats) {
				m_gameLogic.destroyFigure(static_cast<common::ID>(spawned.getIndex()));
			}
			return false;
		}
		botf::log::info(
			"[server] session {} seated client {} → entity/spawn {}", m_id, player->getId(),
			entityId);
	}

	m_phase = SessionPhase::Playing;
	botf::log::info("[server] session {} Forming → Playing ({} players)", m_id, seatCapacity());
	return true;
}

void botf::GameSession::randomShuffleIndex() {
	const std::vector<std::size_t> indices = utils::randomIndices(m_seats.size());
	for (std::size_t i = 0; i < m_seats.size(); ++i) {
		m_seats[i].setIndex(indices[i]);
	}
}

void botf::GameSession::removePlayer(const common::ID clientId) {
	Seat* seat = findSeatByPlayerId(clientId);
	if (!seat) {
		return;
	}
	const common::ID entityId = static_cast<common::ID>(seat->getIndex());
	seat->clearPlayer();
	if (m_phase == SessionPhase::Playing) {
		m_gameLogic.destroyFigure(entityId);
	}
}

void botf::GameSession::applyClientInput(const common::ID clientId,
										const common::ClientInput& clientInput) {
	if (m_phase != SessionPhase::Playing) {
		return;
	}
	const Seat* seat = findSeatByPlayerId(clientId);
	if (!seat) {
		return;
	}
	const common::ID entityId = static_cast<common::ID>(seat->getIndex());
	m_gameLogic.setFigureInput(entityId, clientInput.dx, clientInput.dy);
	m_gameLogic.setFigureAim(entityId, clientInput.aim);
	if (clientInput.fire) {
		(void)m_gameLogic.tryFire(entityId);
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
	const Seat* seat = findSeatByPlayerId(clientId);
	if (!seat) {
		return std::nullopt;
	}
	return static_cast<common::ID>(seat->getIndex());
}
