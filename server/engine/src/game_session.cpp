#include "engine/game_session.h"

#include "common/global_settings.hpp"
#include "common/log.hpp"
#include "utils/random.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

botf::GameSession::GameSession(const common::ID id) : m_id{id} {
	const std::size_t capacity =
		static_cast<std::size_t>(common::GlobalSettings::SessionMode::Default);
	m_seats.reserve(capacity);
	for (common::IDX i = 0; i < static_cast<common::IDX>(capacity); ++i) {
		m_seats.push_back(std::make_shared<Seat>(i));
	}
}

botf::common::ID botf::GameSession::getId() const {
	return m_id;
}

bool botf::GameSession::hasGameMap() const {
	return static_cast<bool>(m_gameMap);
}

const botf::GameMap& botf::GameSession::getGameMap() const {
	if (!m_gameMap) {
		throw std::runtime_error("GameSession::getGameMap: map not assigned (call beginPlaying)");
	}
	return *m_gameMap;
}

botf::SessionState botf::GameSession::getSessionState() const {
	return m_sessionState;
}

std::size_t botf::GameSession::seatCapacity() const {
	return m_seats.size();
}

std::size_t botf::GameSession::clientCount() const {
	std::size_t count = 0;
	for (const std::shared_ptr<Seat>& seat : m_seats) {
		if (!seat->isFree()) {
			++count;
		}
	}
	return count;
}

std::vector<botf::common::ID> botf::GameSession::clients() const {
	std::vector<common::ID> clientIDs;
	clientIDs.reserve(m_seats.size());
	for (const std::shared_ptr<Seat>& seat : m_seats) {
		if (seat->getPlayer()) {
			clientIDs.push_back(seat->getPlayer()->getId());
		}
	}
	return clientIDs;
}

const std::vector<std::shared_ptr<botf::Seat>>& botf::GameSession::seats() const {
	return m_seats;
}

bool botf::GameSession::hasFreeSeats() const {
	if (m_sessionState != SessionState::Forming) {
		return false;
	}
	for (const std::shared_ptr<Seat>& seat : m_seats) {
		if (seat->isFree()) {
			return true;
		}
	}
	return false;
}

std::shared_ptr<botf::Seat> botf::GameSession::findFreeSeat() {
	for (std::shared_ptr<Seat>& seat : m_seats) {
		if (seat->isFree()) {
			return seat;
		}
	}
	return nullptr;
}

std::shared_ptr<botf::Seat> botf::GameSession::findSeatByPlayerId(const common::ID clientId) {
	for (std::shared_ptr<Seat>& seat : m_seats) {
		if (seat->getPlayer() && seat->getPlayer()->getId() == clientId) {
			return seat;
		}
	}
	return nullptr;
}

std::shared_ptr<botf::Seat> botf::GameSession::findSeatByPlayerId(const common::ID clientId) const {
	for (const std::shared_ptr<Seat>& seat : m_seats) {
		if (seat->getPlayer() && seat->getPlayer()->getId() == clientId) {
			return seat;
		}
	}
	return nullptr;
}

bool botf::GameSession::addPlayer(std::shared_ptr<Player> player, const common::ID figureTypeId) {
	if (!player) {
		return false;
	}
	if (std::shared_ptr<Seat> existing = findSeatByPlayerId(player->getId())) {
		existing->setFigureTypeId(figureTypeId);
		return true;
	}
	if (m_sessionState != SessionState::Forming) {
		return false;
	}
	const std::shared_ptr<Seat> seat = findFreeSeat();
	if (!seat) {
		return false;
	}
	seat->setFigureTypeId(figureTypeId);
	seat->setPlayer(std::move(player));
	return true;
}

bool botf::GameSession::beginPlaying(std::shared_ptr<BootstrapStorage> storage) {
	if (!storage) {
		botf::log::error("[server] beginPlaying refused: session {} storage is null", m_id);
		return false;
	}
	if (m_sessionState != SessionState::Forming) {
		botf::log::warn("[server] beginPlaying refused: session {} state != Forming", m_id);
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
	if (storage->gameMapPool().empty() || storage->figureDescriptionPool().empty()) {
		botf::log::error("[server] beginPlaying: session {} BootstrapStorage catalogs empty", m_id);
		return false;
	}

	// Resolve figure templates before mutating seats / map.
	std::vector<std::shared_ptr<const FigureDescription>> figureDescs;
	figureDescs.reserve(m_seats.size());
	for (const std::shared_ptr<Seat>& seat : m_seats) {
		const auto figureDesc =
			storage->figureDescriptionPool().getById(seat->getFigureTypeId());
		if (!figureDesc) {
			botf::log::error("[server] beginPlaying: session {} unknown figure type {}", m_id,
							 seat->getFigureTypeId());
			return false;
		}
		figureDescs.push_back(figureDesc);
	}

	const std::shared_ptr<const GameMap> gameMap = storage->gameMapPool().pickRandomGameMap();
	if (!gameMap || gameMap->getSpawns().size() < seatCapacity()) {
		botf::log::error(
			"[server] beginPlaying: session {} map spawns={} < capacity={}", m_id,
			gameMap ? gameMap->getSpawns().size() : 0u, seatCapacity());
		return false;
	}

	// Spawn using a permutation; commit seat indices only after every spawn succeeds.
	const std::vector<std::size_t> spawnSlots = utils::randomIndices(m_seats.size());
	const std::vector<common::Point>& spawns = gameMap->getSpawns();
	m_gameMap = gameMap;

	for (std::size_t i = 0; i < m_seats.size(); ++i) {
		const common::ID entityId = static_cast<common::ID>(spawnSlots[i]);
		if (!m_gameLogic.spawnFigure(entityId, spawns[spawnSlots[i]], *figureDescs[i])) {
			botf::log::error(
				"[server] beginPlaying: session {} failed to spawn figure id {}", m_id, entityId);
			for (std::size_t j = 0; j < i; ++j) {
				m_gameLogic.destroyFigure(static_cast<common::ID>(spawnSlots[j]));
			}
			m_gameMap.reset();
			return false;
		}
		botf::log::info(
			"[server] session {} seated client {} → entity/spawn {} figureType={}", m_id,
			m_seats[i]->getPlayer()->getId(), entityId, m_seats[i]->getFigureTypeId());
	}

	for (std::size_t i = 0; i < m_seats.size(); ++i) {
		m_seats[i]->setIndex(static_cast<common::IDX>(spawnSlots[i]));
	}

	m_sessionState = SessionState::Playing;
	botf::log::info("[server] session {} Forming → Playing ({} players, mapId={})", m_id,
					seatCapacity(), m_gameMap->getId());
	return true;
}

void botf::GameSession::removePlayer(const common::ID clientId) {
	const std::shared_ptr<Seat> seat = findSeatByPlayerId(clientId);
	if (!seat) {
		return;
	}
	seat->clearPlayer();
	if (m_sessionState == SessionState::Playing) {
		m_gameLogic.destroyFigure(static_cast<common::ID>(seat->getIndex()));
	}
}

void botf::GameSession::applyClientInput(const common::ID clientId,
										const common::ClientInput& clientInput) {
	if (m_sessionState != SessionState::Playing) {
		return;
	}
	const std::shared_ptr<Seat> seat = findSeatByPlayerId(clientId);
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
	if (m_sessionState != SessionState::Playing) {
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

bool botf::GameSession::hasClient(const common::ID clientId) const {
	return findSeatByPlayerId(clientId) != nullptr;
}

std::optional<botf::common::ID> botf::GameSession::figureIdForClient(
	const common::ID clientId) const {
	const std::shared_ptr<Seat> seat = findSeatByPlayerId(clientId);
	if (!seat) {
		return std::nullopt;
	}
	return static_cast<common::ID>(seat->getIndex());
}
