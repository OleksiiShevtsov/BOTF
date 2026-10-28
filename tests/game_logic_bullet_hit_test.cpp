#include "common/dto.hpp"
#include "common/global_settings.hpp"
#include "helpers/test_env.h"
#include "logic/hitbox.h"

#include <cmath>

#include <gtest/gtest.h>

namespace {

constexpr float kDt = 1.0f / 20.0f;

botf::common::ID spawnBound(botf::test::TestGameSession& session,
						   botf::common::ID clientId,
						   botf::common::Point at) {
	const auto id = session.spawnPlayer(at);
	session.bindClient(clientId, id);
	return id;
}

const botf::FigureObject* findFigure(const botf::GameSession& session, botf::common::ID id) {
	for (const auto& f : session.figures()) {
		if (f.getId() == id) {
			return &f;
		}
	}
	return nullptr;
}

}  // namespace

TEST(GameLogicBulletHit, HitsForeignFigureAppliesDamageAndDestroysBullet) {
	botf::test::TestSession env;
	auto& session = env.session;
	const auto shooterId = spawnBound(session, 1, {0.f, 0.f});
	const auto targetId = spawnBound(session, 2, {50.f, 0.f});
	session.applyClientInput(1, botf::common::ClientInput{0, 0, {100.f, 0.f}, true, 0});

	ASSERT_EQ(session.bullets().size(), 1u);

	const int hpBefore = findFigure(session, targetId)->getHp();
	bool hit = false;
	for (int i = 0; i < 20; ++i) {
		session.tick(kDt);
		if (session.bullets().empty()) {
			hit = true;
			break;
		}
	}
	ASSERT_TRUE(hit) << "bullet should hit target within range";
	const auto* target = findFigure(session, targetId);
	ASSERT_NE(target, nullptr);
	EXPECT_EQ(target->getHp(), hpBefore - botf::common::GlobalSettings::bulletDamage);
	EXPECT_EQ(findFigure(session, shooterId)->getHp(), botf::common::GlobalSettings::maxHp);
}

TEST(GameLogicBulletHit, MissJustOutsideLeavesHpAndBulletMayExpire) {
	botf::test::TestSession env;
	auto& session = env.session;
	spawnBound(session, 1, {0.f, 0.f});
	const auto targetId = spawnBound(session, 2, {50.f, 80.f});
	session.applyClientInput(1, botf::common::ClientInput{0, 0, {100.f, 0.f}, true, 0});
	const int hpBefore = findFigure(session, targetId)->getHp();

	const float maxRange = botf::common::GlobalSettings::bulletMaxRange;
	const float speed = botf::common::GlobalSettings::bulletSpeed;
	const int ticksNeeded = static_cast<int>(std::ceil(maxRange / (speed * kDt))) + 2;
	for (int i = 0; i < ticksNeeded; ++i) {
		session.tick(kDt);
	}
	EXPECT_TRUE(session.bullets().empty());
	EXPECT_EQ(findFigure(session, targetId)->getHp(), hpBefore);
}

TEST(GameLogicBulletHit, OwnBulletDoesNotDamageShooter) {
	botf::test::TestSession env;
	auto& session = env.session;
	const auto shooterId = spawnBound(session, 1, {0.f, 0.f});
	session.applyClientInput(1, botf::common::ClientInput{0, 0, {100.f, 0.f}, true, 0});

	for (int i = 0; i < 3; ++i) {
		session.tick(kDt);
		EXPECT_EQ(findFigure(session, shooterId)->getHp(), botf::common::GlobalSettings::maxHp);
	}
}

TEST(GameLogicBulletHit, PointInsideTargetAabbIsHit) {
	const botf::common::Point center{50.f, 0.f};
	const botf::ObjectHitbox box = botf::ObjectHitbox::installDefault();
	EXPECT_TRUE(box.containsPoint(center, {50.f, 0.f}));
	EXPECT_FALSE(box.containsPoint(center, {50.f, box.getHalfHeight() + 0.01f}));
}
