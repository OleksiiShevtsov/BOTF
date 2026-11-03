#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace botf::utils {

// Shared RNG for server/client helpers (maps, spawns, …). Not crypto-secure.
void seedRandom(std::uint32_t seed);

// Uniform index in [0, count). Throws if count == 0.
[[nodiscard]] std::size_t randomIndex(std::size_t count);

// Permutation of [0, count): Fisher–Yates via randomIndex. Empty if count == 0.
[[nodiscard]] std::vector<std::size_t> randomIndices(std::size_t count);

}  // namespace botf::utils
