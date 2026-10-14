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

}  // namespace botf::common::GlobalSettings
