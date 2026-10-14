#include "app/match.h"

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

void botf::app::Match::applyWorldSnapshot(const common::WorldSnapshot& snapshot) {
	if (!m_running) {
		start();
	}
	m_scene.upsertFromSnapshot(snapshot);
}

void botf::app::Match::draw(sf::RenderTarget& target) const {
	m_map.draw(target);
	m_scene.draw(target);
}

botf::app::GameMap& botf::app::Match::map() {
	return m_map;
}

const botf::app::GameMap& botf::app::Match::map() const {
	return m_map;
}

botf::app::Scene& botf::app::Match::scene() {
	return m_scene;
}

const botf::app::Scene& botf::app::Match::scene() const {
	return m_scene;
}
