#pragma once

#include "logic/dynamic_object.h"

namespace botf {

// Server bullet: point mass, constant velocity, dies after max travel or on hit.
// Client draws a temporary segment; hit-test uses this point only (card 41).
class BulletObject : public DynamicObject {
public:
	BulletObject(common::ObjectID id,
				 common::ObjectID ownerId,
				 common::Point origin,
				 common::Angle direction,
				 float speed,
				 float maxRange);

	[[nodiscard]] common::Angle getAngle() const override;
	[[nodiscard]] common::ObjectID getOwnerId() const;
	[[nodiscard]] common::Point getVelocity() const;
	[[nodiscard]] bool isAlive() const override;

	// Mark for prune after hit (or other server destroy).
	void destroy();

	void update(float dt) override;

private:
	common::ObjectID m_ownerId{};
	common::Angle m_angle{};
	common::Point m_velocity{};
	float m_maxRange{};
	float m_distanceTraveled{0.f};
	bool m_alive{true};
};

}  // namespace botf
