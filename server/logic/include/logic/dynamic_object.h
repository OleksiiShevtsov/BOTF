#pragma once

#include "common/types.hpp"

namespace botf {

// Movable entity with acceleration / inertia (from old DynamicObject, adapted to dx/dy + dt).
class DynamicObject {
public:
	DynamicObject(common::ID id, common::Point dot, float speedLimit);

	[[nodiscard]] common::ID id() const;
	[[nodiscard]] common::Point position() const;
	[[nodiscard]] common::Angle angle() const;
	[[nodiscard]] int inputDx() const;
	[[nodiscard]] int inputDy() const;

	void setPosition(common::Point dot);
	void setAngle(common::Angle alpha);

	// Movement intent for this tick: each axis in {-1, 0, 1}.
	void setInput(int dx, int dy);

	// Advance simulation by fixed dt (seconds).
	void update(float dt);

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

	common::ID m_id;
	common::Point m_position{};
	common::Angle m_angle{0.0f};
	float m_speedLimit;

	int m_inputDx{0};
	int m_inputDy{0};
	int m_prevInputDx{0};
	int m_prevInputDy{0};

	MovementParameters m_movement{};
};

}  // namespace botf
