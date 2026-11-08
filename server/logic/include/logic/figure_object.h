#pragma once

#include "logic/dynamic_object.h"
#include "logic/hitbox.h"

namespace botf {

// Controllable figure with incremental accel / friction (card 36).
class FigureObject : public DynamicObject {
public:
	// Defaults match content/figures/1 for unit tests that spawn without a catalog.
	FigureObject(const common::ID id,
				 common::Point dot,
				 float speedLimit,
				 ObjectHitbox hitbox = ObjectHitbox::installDefault(),
				 float acceleration = 600.f,
				 float friction = 400.f);

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
	// Store aim world point and retarget angle now. No-op if dead.
	void setAimWorld(common::Point aimWorld);
	// Fire cooldown gate. true → caller may spawn a bullet. No-op/false if dead.
	[[nodiscard]] bool tryFire(float worldTime);
	// Subtract damage, clamp hp >= 0. Returns true if this call reached Dead.
	bool applyDamage(int damage);

	void update(float dt) override;

private:
	void clampSpeed();
	void retargetFromAim();

	common::Angle m_angle{0.0f};
	float m_speedLimit{};
	float m_acceleration{};
	float m_friction{};
	int m_hp{};
	common::LifeStatus m_lifeStatus{common::LifeStatus::Alive};
	ObjectHitbox m_hitbox{};

	int m_inputDx{0};
	int m_inputDy{0};

	float m_speedX{0.f};
	float m_speedY{0.f};

	common::Point m_aimWorld{};
	bool m_hasAimWorld{false};
	float m_lastFireTime{0.f};
	bool m_hasLastFire{false};
};

}  // namespace botf
