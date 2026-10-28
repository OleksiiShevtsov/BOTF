#pragma once

#include <cstddef>
#include <cstdint>

namespace botf::utils {

// Shared RNG for server/client helpers (maps, spawns, …). Not crypto-secure.
void seedRandom(std::uint32_t seed);

// Uniform index in [0, count). Throws if count == 0.
[[nodiscard]] std::size_t randomIndex(std::size_t count);

}  // namespace botf::utils
