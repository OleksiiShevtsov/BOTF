#pragma once

#include "common/types.hpp"
#include "logic/figure_spawn_params.h"
#include "logic/hitbox.h"
#include "utils/figure_loader.hpp"

#include <string>

namespace botf {

// Immutable figure template from content/figures/<id>/figure.json.
class FigureDescription {
public:
	explicit FigureDescription(const utils::FigureData& data)
		: m_id{data.id},
		  m_name{data.name},
		  m_params{ObjectHitbox{data.hitboxWidth, data.hitboxHeight, data.hitboxOffset.x,
								data.hitboxOffset.y},
				   data.acceleration, data.friction, data.speedLimit} {}

	[[nodiscard]] common::ID getId() const {
		return m_id;
	}
	[[nodiscard]] const std::string& getName() const {
		return m_name;
	}
	[[nodiscard]] const FigureSpawnParams& getSpawnParams() const {
		return m_params;
	}
	[[nodiscard]] const ObjectHitbox& getHitbox() const {
		return m_params.hitbox;
	}
	[[nodiscard]] float getAcceleration() const {
		return m_params.acceleration;
	}
	[[nodiscard]] float getFriction() const {
		return m_params.friction;
	}
	[[nodiscard]] float getSpeedLimit() const {
		return m_params.speedLimit;
	}

private:
	common::ID m_id{};
	std::string m_name;
	FigureSpawnParams m_params{};
};

}  // namespace botf
