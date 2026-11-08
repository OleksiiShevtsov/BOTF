#include "engine/bootstrap_storage.h"

bool botf::BootstrapStorage::load() {
	const bool mapsOk = m_gameMapPool.load();
	const bool figuresOk = m_figureDescriptionPool.load();
	return mapsOk && figuresOk;
}

botf::GameMapPool& botf::BootstrapStorage::gameMapPool() {
	return m_gameMapPool;
}

const botf::GameMapPool& botf::BootstrapStorage::gameMapPool() const {
	return m_gameMapPool;
}

botf::FigureDescriptionPool& botf::BootstrapStorage::figureDescriptionPool() {
	return m_figureDescriptionPool;
}

const botf::FigureDescriptionPool& botf::BootstrapStorage::figureDescriptionPool() const {
	return m_figureDescriptionPool;
}
