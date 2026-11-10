#pragma once

#include "common/types.hpp"
#include "engine/figure_description.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace botf {

// Catalog of figure templates from content/figures. Owned by BootstrapStorage.
class FigureDescriptionPool {
public:
	FigureDescriptionPool() = default;

	FigureDescriptionPool(const FigureDescriptionPool&) = delete;
	FigureDescriptionPool& operator=(const FigureDescriptionPool&) = delete;

	void clear();

	// Scan content/figures. false if none loaded or any figure.json failed.
	bool load();

	[[nodiscard]] bool empty() const;
	[[nodiscard]] std::size_t size() const;

	// From cache (after load()). nullptr if unknown id.
	[[nodiscard]] std::shared_ptr<const FigureDescription> getById(
		common::ID figureTypeId) const;

private:
	std::vector<std::shared_ptr<const FigureDescription>> m_figures;
};

}  // namespace botf
