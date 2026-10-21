#pragma once

#include "common/life_status.hpp"
#include "logic/dynamic_object.h"

namespace botf {

// Controllable figure with acceleration / inertia (former DynamicObject).
class FigureObject : public DynamicObject {
public:
	FigureObject(common::ObjectID id, common::Point dot, float speedLimit);

	[[nodiscard]] common::Angle getAngle() const override;
	[[nodiscard]] int inputDx() const;
	[[nodiscard]] int inputDy() const;
	[[nodiscard]] int getHp() const;
	[[nodiscard]] common::LifeStatus getLifeStatus() const;
	[[nodiscard]] bool isDead() const;

	void setAngle(common::Angle alpha);
	// Movement intent for this tick: each axis in {-1, 0, 1}. No-op if dead.
	void setInput(int dx, int dy);
	// Subtract damage, clamp hp >= 0. Returns true if this call reached Dead.
	bool applyDamage(int damage);

	void update(float dt) override;

private:
	struct MovementParameters {
		float cosAcceleration{};
		float sinAcceleration{};
		float speedX{};
		float speedY{};
		float initialSpeedX{};
		float initialSpeedY{};
		common::Coordinate initialX{};
		common::Coordinate initialY{};
		float currentTime{};
	};

	float speedRecalculation(float acceleration, float initialSpeed) const;
	common::Coordinate coordinateRecalculation(float acceleration,
											   float initialSpeed,
											   common::Coordinate initialCoordinate) const;
	void resetEndpoints();

	static constexpr float kSpeedEpsilon{2.0f};
	static constexpr float kAcceleration{50.0f};
	static constexpr float kFriction{-9.81f / 0.4f};

	common::Angle m_angle{0.0f};
	float m_speedLimit{};
	int m_hp{};
	common::LifeStatus m_lifeStatus{common::LifeStatus::Alive};

	int m_inputDx{0};
	int m_inputDy{0};
	int m_prevInputDx{0};
	int m_prevInputDy{0};

	MovementParameters m_movement{};
};

}  // namespace botf
