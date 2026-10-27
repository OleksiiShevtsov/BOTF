#pragma once

#include "logic/dynamic_object.h"
#include "logic/hitbox.h"

namespace botf {

// Controllable figure with incremental accel / friction (card 36).
class FigureObject : public DynamicObject {
public:
	// hitbox: stub default from GlobalSettings until character catalog / DB (card 50).
	FigureObject(const common::ID id,
				 common::Point dot,
				 float speedLimit,
				 ObjectHitbox hitbox = ObjectHitbox::installDefault());

	[[nodiscard]] common::Angle getAngle() const override;
	[[nodiscard]] int inputDx() const;
	[[nodiscard]] int inputDy() const;
	[[nodiscard]] int getHp() const;
	[[nodiscard]] common::LifeStatus getLifeStatus() const;
	[[nodiscard]] bool isDead() const;
	[[nodiscard]] common::Point getVelocity() const;
	[[nodiscard]] const ObjectHitbox& getHitbox() const;

	void setAngle(common::Angle alpha);
	// Movement intent for this tick: each axis in {-1, 0, 1}. No-op if dead.
	void setInput(int dx, int dy);
	// Subtract damage, clamp hp >= 0. Returns true if this call reached Dead.
	bool applyDamage(int damage);

	void update(float dt) override;

private:
	void clampSpeed();

	common::Angle m_angle{0.0f};
	float m_speedLimit{};
	int m_hp{};
	common::LifeStatus m_lifeStatus{common::LifeStatus::Alive};
	ObjectHitbox m_hitbox{};

	int m_inputDx{0};
	int m_inputDy{0};

	float m_speedX{0.f};
	float m_speedY{0.f};
};

}  // namespace botf
