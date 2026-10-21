#pragma once

#include <chrono>
#include <cstddef>

namespace botf::common::GlobalSettings {

constexpr std::size_t sizeWindowX{1080};
constexpr std::size_t sizeWindowY{640};
constexpr std::chrono::milliseconds pauseBetweenButtonPresses{50};

// Default gameplay tuning (server logic).
constexpr float defaultPlayerSpeedLimit{200.0f};
constexpr float defaultSpawnX{100.0f};
constexpr float defaultSpawnY{300.0f};
// Ignore aim updates when the cursor is this close to the entity (world units).
constexpr float aimEpsilon{1.0f};
// Client Aim send: fire every frame when angle or point moves past these.
constexpr float aimSendAngleEpsilon{0.03f};	 // ~1.7°
constexpr float aimSendPointDelta{0.25f};

// Combat / bullets — TODO: load from DB later.
constexpr float fireCooldownSeconds{0.5f};
constexpr float bulletSpeed{400.0f};		// world units / second
constexpr float bulletMaxRange{100.0f};		// destroy after this travel distance
constexpr float bulletVisualLength{20.0f};	// client draw only (temporary)
constexpr float bulletVisualWidth{4.0f};	// client draw only (temporary)
// Player AABB hitbox (axis-aligned square, center = entity pos). Card 34.
constexpr float hitboxSize{40.0f};
// Player HP / combat (cards 32–33). TODO: load from DB later.
constexpr int maxHp{100};
constexpr int bulletDamage{20};

}  // namespace botf::common::GlobalSettings

