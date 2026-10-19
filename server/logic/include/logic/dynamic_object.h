#pragma once

#include "common/types.hpp"

namespace botf {

// Abstract sim entity (figures, bullets, …). No networking / SFML.
class DynamicObject {
public:
	virtual ~DynamicObject() = default;

	[[nodiscard]] common::ObjectID id() const;
	[[nodiscard]] common::Point position() const;
	[[nodiscard]] virtual common::Angle getAngle() const = 0;

	void setPosition(common::Point dot);

	// Advance simulation by fixed dt (seconds).
	virtual void update(float dt) = 0;

	// Expired entities (e.g. bullets past max range) are removed after tick.
	[[nodiscard]] virtual bool isAlive() const;

protected:
	DynamicObject(common::ObjectID id, common::Point position);

	common::ObjectID m_id{};
	common::Point m_position{};
};

}  // namespace botf
