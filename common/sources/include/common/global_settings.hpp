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
constexpr float aimSendAngleEpsilon{0.03f};  // ~1.7°
constexpr float aimSendPointDelta{0.25f};

}  // namespace botf::common::GlobalSettings
