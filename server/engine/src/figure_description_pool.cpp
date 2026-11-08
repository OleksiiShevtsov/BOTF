#include "engine/figure_description_pool.h"

#include "utils/figure_loader.hpp"

#include <stdexcept>

void botf::FigureDescriptionPool::clear() {
	m_figures.clear();
}

bool botf::FigureDescriptionPool::load() {
	clear();
	for (const common::ID figureId : utils::listFigureIds()) {
		const auto data = utils::loadFigure(figureId);
		if (!data) {
			throw std::runtime_error("FigureDescriptionPool::load: failed to load figure id " +
									 std::to_string(figureId));
		}
		m_figures.push_back(std::make_shared<const FigureDescription>(*data));
	}
	return !m_figures.empty();
}

bool botf::FigureDescriptionPool::empty() const {
	return m_figures.empty();
}

std::size_t botf::FigureDescriptionPool::size() const {
	return m_figures.size();
}

botf::common::ID botf::FigureDescriptionPool::firstFigureTypeId() const {
	if (m_figures.empty()) {
		throw std::runtime_error("FigureDescriptionPool::firstFigureTypeId: pool is empty");
	}
	return m_figures.front()->getId();
}

std::shared_ptr<const botf::FigureDescription> botf::FigureDescriptionPool::getById(
	const common::ID figureTypeId) const {
	for (const auto& figure : m_figures) {
		if (figure && figure->getId() == figureTypeId) {
			return figure;
		}
	}
	throw std::runtime_error("FigureDescriptionPool::getById: unknown figure type id " +
							 std::to_string(figureTypeId));
}
