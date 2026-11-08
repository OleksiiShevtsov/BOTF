#pragma once

#include <chrono>
#include <cstddef>

namespace botf::common::GlobalSettings {

constexpr std::size_t sizeWindowX{1080};
constexpr std::size_t sizeWindowY{640};
constexpr std::chrono::milliseconds pauseBetweenButtonPresses{50};

// Temporary match capacity until map drives seat count at beginPlaying.
enum class SessionMode : std::size_t {
	Default = 2,
};

// Default gameplay tuning (server logic).
constexpr float defaultPlayerSpeedLimit{200.0f};
// Spawns / map bounds live in content/maps/<id>/map.json (card 51), not here.
// Acceleration / friction live in content/figures/<id>/figure.json.
// Ignore aim updates when the cursor is this close to the entity (world units).
constexpr float aimEpsilon{1.0f};
// Client Aim send: fire every frame when angle or point moves past these.
constexpr float aimSendAngleEpsilon{0.03f};	 // ~1.7°
constexpr float aimSendPointDelta{0.25f};

// Combat — TODO: нет БД, load from DB later.
constexpr float fireCooldownSeconds{0.1f};
constexpr float bulletSpeed{400.0f};		// world units / second
constexpr float bulletMaxRange{500.0f};		// destroy after this travel distance
constexpr float bulletVisualLength{20.0f};	// client draw only (temporary)
constexpr float bulletVisualWidth{4.0f};	// client draw only (temporary)
// Default AABB square for ObjectHitbox stub (card 50) — TODO: нет БД / character catalog.
constexpr float hitboxSize{40.0f};			// width=height; center = entity pos
constexpr int maxHp{100};
constexpr int bulletDamage{20};

// Server tick rate and timing (20 Hz, 50 ms per tick). This is the authoritative simulation rate.
constexpr int kTickHz = 20;
constexpr float kDt = 1.0f / static_cast<float>(kTickHz);
constexpr auto kTickPeriod = std::chrono::milliseconds(1000 / kTickHz);
}  // namespace botf::common::GlobalSettings

