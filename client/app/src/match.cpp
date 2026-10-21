#include "app/match.h"

#include "common/global_settings.hpp"

void botf::app::Match::start() {
	m_running = true;
}

void botf::app::Match::update() {
	// Presentation tick hook (prediction / FX later). Snapshot apply is separate.
}

void botf::app::Match::finish() {
	m_running = false;
	m_scene.clear();
}

void botf::app::Match::applyWorldSnapshot(const common::WorldSnapshot& snapshot,
										  std::optional<common::ObjectID> selfId) {
	if (!m_running) {
		start();
	}
	m_scene.upsertFromSnapshot(snapshot);

	if (!selfId) {
		return;
	}
	for (const auto& pose : snapshot.players) {
		if (pose.id == *selfId) {
			m_infoPanel.setHp(pose.hp, common::GlobalSettings::maxHp);
			return;
		}
	}
}

void botf::app::Match::draw(sf::RenderTarget& target) const {
	m_map.draw(target);
	m_scene.draw(target);
	m_infoPanel.draw(target);
}

botf::app::GameMap& botf::app::Match::getMap() {
	return m_map;
}

const botf::app::GameMap& botf::app::Match::getMap() const {
	return m_map;
}

botf::app::Scene& botf::app::Match::getScene() {
	return m_scene;
}

const botf::app::Scene& botf::app::Match::getScene() const {
	return m_scene;
}

const botf::app::InformationPanel& botf::app::Match::getInformationPanel() const {
	return m_infoPanel;
}
