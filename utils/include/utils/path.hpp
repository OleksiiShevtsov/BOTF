#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace botf::utils {

// Resolve content root: BOTF_CONTENT_ROOT env, else BOTF_DEFAULT_CONTENT_ROOT (CMake),
// else <cwd>/content.
[[nodiscard]] std::filesystem::path contentRoot();

// Override for tests (empty clears override and falls back to env/default).
void setContentRootOverride(std::filesystem::path root);

[[nodiscard]] std::filesystem::path mapDir(const std::filesystem::path& root,
										   std::uint32_t mapId);
[[nodiscard]] std::filesystem::path mapJsonPath(const std::filesystem::path& root,
												std::uint32_t mapId);

}  // namespace botf::utils
