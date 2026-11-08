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

	// Scan content/figures. Returns false if none loaded.
	bool load();

	[[nodiscard]] bool empty() const;
	[[nodiscard]] std::size_t size() const;

	// First loaded template id (stable: sorted by id). Throws if empty.
	[[nodiscard]] common::ID firstFigureTypeId() const;

	// From cache (after load()). Throws if unknown id.
	[[nodiscard]] std::shared_ptr<const FigureDescription> getById(
		const common::ID figureTypeId) const;

private:
	std::vector<std::shared_ptr<const FigureDescription>> m_figures;
};

}  // namespace botf
