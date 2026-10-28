#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "helpers/test_env.h"
#include "logic/game_logic.h"
#include "logic/hitbox.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

const botf::FigureObject* findFigure(const botf::GameLogic& logic, botf::common::ID id) {
	for (const auto& f : logic.figures()) {
		if (f.getId() == id) {
			return &f;
		}
	}
	return nullptr;
}

}  // namespace

TEST(GameLogicBulletHit, HitsForeignFigureAppliesDamageAndDestroysBullet) {
	botf::GameLogic logic;
	const auto shooterId = botf::test::spawnBound(logic, 1, {0.f, 0.f});
	const auto targetId = botf::test::spawnBound(logic, 2, {50.f, 0.f});
	botf::test::applyClientInput(logic, 1, botf::common::ClientInput{0, 0, {100.f, 0.f}, true, 0});

	ASSERT_EQ(logic.bullets().size(), 1u);

	const int hpBefore = findFigure(logic, targetId)->getHp();
	bool hit = false;
	for (int i = 0; i < 20; ++i) {
		logic.tick(kDt);
		if (logic.bullets().empty()) {
			hit = true;
			break;
		}
	}
	ASSERT_TRUE(hit) << "bullet should hit target within range";
	const auto* target = findFigure(logic, targetId);
	ASSERT_NE(target, nullptr);
	EXPECT_EQ(target->getHp(), hpBefore - botf::common::GlobalSettings::bulletDamage);
	EXPECT_EQ(findFigure(logic, shooterId)->getHp(), botf::common::GlobalSettings::maxHp);
}

TEST(GameLogicBulletHit, MissJustOutsideLeavesHpAndBulletMayExpire) {
	botf::GameLogic logic;
	botf::test::spawnBound(logic, 1, {0.f, 0.f});
	const auto targetId = botf::test::spawnBound(logic, 2, {50.f, 80.f});
	botf::test::applyClientInput(logic, 1, botf::common::ClientInput{0, 0, {100.f, 0.f}, true, 0});
	const int hpBefore = findFigure(logic, targetId)->getHp();

	const float maxRange = botf::common::GlobalSettings::bulletMaxRange;
	const float speed = botf::common::GlobalSettings::bulletSpeed;
	const int ticksNeeded = static_cast<int>(std::ceil(maxRange / (speed * kDt))) + 2;
	for (int i = 0; i < ticksNeeded; ++i) {
		logic.tick(kDt);
	}
	EXPECT_TRUE(logic.bullets().empty());
	EXPECT_EQ(findFigure(logic, targetId)->getHp(), hpBefore);
}

TEST(GameLogicBulletHit, OwnBulletDoesNotDamageShooter) {
	botf::GameLogic logic;
	const auto shooterId = botf::test::spawnBound(logic, 1, {0.f, 0.f});
	botf::test::applyClientInput(logic, 1, botf::common::ClientInput{0, 0, {100.f, 0.f}, true, 0});

	for (int i = 0; i < 3; ++i) {
		logic.tick(kDt);
		EXPECT_EQ(findFigure(logic, shooterId)->getHp(), botf::common::GlobalSettings::maxHp);
	}
}

TEST(GameLogicBulletHit, PointInsideTargetAabbIsHit) {
	const botf::common::Point center{50.f, 0.f};
	const botf::ObjectHitbox box = botf::ObjectHitbox::installDefault();
	EXPECT_TRUE(box.containsPoint(center, {50.f, 0.f}));
	EXPECT_FALSE(box.containsPoint(center, {50.f, box.getHalfHeight() + 0.01f}));
}
