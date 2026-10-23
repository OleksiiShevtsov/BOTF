#include "common/global_settings.hpp"
#include "logic/game_logic.h"
#include "logic/hitbox.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

botf::common::ObjectID spawnBound(botf::GameLogic& game,
								  botf::common::ClientID clientId,
								  botf::common::Point at) {
	const auto id = game.spawnPlayer(at, botf::common::GlobalSettings::defaultPlayerSpeedLimit);
	game.bindClient(clientId, id);
	return id;
}

const botf::FigureObject* findFigure(const botf::GameLogic& game, botf::common::ObjectID id) {
	for (const auto& f : game.figures()) {
		if (f.getObjectId() == id) {
			return &f;
		}
	}
	return nullptr;
}

}  // namespace

TEST(GameLogicBulletHit, HitsForeignFigureAppliesDamageAndDestroysBullet) {
	botf::GameLogic game;
	const auto shooterId = spawnBound(game, 1, {0.f, 0.f});
	const auto targetId = spawnBound(game, 2, {50.f, 0.f});
	game.setClientAim(1, {100.f, 0.f});

	ASSERT_TRUE(game.tryFire(1));
	ASSERT_EQ(game.bullets().size(), 1u);

	const int hpBefore = findFigure(game, targetId)->getHp();
	bool hit = false;
	for (int i = 0; i < 20; ++i) {
		game.tick(kDt);
		if (game.bullets().empty()) {
			hit = true;
			break;
		}
	}
	ASSERT_TRUE(hit) << "bullet should hit target within range";
	const auto* target = findFigure(game, targetId);
	ASSERT_NE(target, nullptr);
	EXPECT_EQ(target->getHp(), hpBefore - botf::common::GlobalSettings::bulletDamage);
	EXPECT_EQ(findFigure(game, shooterId)->getHp(), botf::common::GlobalSettings::maxHp);
}

TEST(GameLogicBulletHit, MissJustOutsideLeavesHpAndBulletMayExpire) {
	botf::GameLogic game;
	spawnBound(game, 1, {0.f, 0.f});
	const auto targetId = spawnBound(game, 2, {50.f, 80.f});
	game.setClientAim(1, {100.f, 0.f});

	ASSERT_TRUE(game.tryFire(1));
	const int hpBefore = findFigure(game, targetId)->getHp();

	const float maxRange = botf::common::GlobalSettings::bulletMaxRange;
	const float speed = botf::common::GlobalSettings::bulletSpeed;
	const int ticksNeeded = static_cast<int>(std::ceil(maxRange / (speed * kDt))) + 2;
	for (int i = 0; i < ticksNeeded; ++i) {
		game.tick(kDt);
	}
	EXPECT_TRUE(game.bullets().empty());
	EXPECT_EQ(findFigure(game, targetId)->getHp(), hpBefore);
}

TEST(GameLogicBulletHit, OwnBulletDoesNotDamageShooter) {
	botf::GameLogic game;
	const auto shooterId = spawnBound(game, 1, {0.f, 0.f});
	game.setClientAim(1, {100.f, 0.f});
	ASSERT_TRUE(game.tryFire(1));

	// First ticks: bullet still inside own AABB — must not self-hit.
	for (int i = 0; i < 3; ++i) {
		game.tick(kDt);
		EXPECT_EQ(findFigure(game, shooterId)->getHp(), botf::common::GlobalSettings::maxHp);
	}
}

TEST(GameLogicBulletHit, PointInsideTargetAabbIsHit) {
	const botf::common::Point center{50.f, 0.f};
	const float half = botf::hitboxHalfExtent();
	EXPECT_TRUE(botf::pointHitsAabb(center, {50.f, 0.f}));
	EXPECT_FALSE(botf::pointHitsAabb(center, {50.f, half + 0.01f}));
}
