#pragma once

#include "common/types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace botf::utils {

// Plain data from content/figures/<id>/figure.json (server slice).
struct FigureData {
	common::ID id{};
	std::string name;
	float hitboxWidth{0.f};
	float hitboxHeight{0.f};
	common::Point hitboxOffset{};
};

[[nodiscard]] std::vector<common::ID> listFigureIds();

[[nodiscard]] std::optional<FigureData> loadFigure(common::ID figureId);

}  // namespace botf::utils
