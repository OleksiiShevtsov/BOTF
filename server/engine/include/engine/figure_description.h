#pragma once

#include "common/types.hpp"
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
		  m_hitbox{data.hitboxWidth, data.hitboxHeight, data.hitboxOffset.x, data.hitboxOffset.y},
		  m_acceleration{data.acceleration},
		  m_friction{data.friction} {}

	[[nodiscard]] common::ID getId() const {
		return m_id;
	}
	[[nodiscard]] const std::string& getName() const {
		return m_name;
	}
	[[nodiscard]] const ObjectHitbox& getHitbox() const {
		return m_hitbox;
	}
	[[nodiscard]] float getAcceleration() const {
		return m_acceleration;
	}
	[[nodiscard]] float getFriction() const {
		return m_friction;
	}

private:
	common::ID m_id{};
	std::string m_name;
	ObjectHitbox m_hitbox{};
	float m_acceleration{};
	float m_friction{};
};

}  // namespace botf
