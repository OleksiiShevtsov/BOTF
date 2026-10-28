#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace botf::utils {

// Read entire file as text. nullopt if missing / unreadable.
[[nodiscard]] std::optional<std::string> readFile(const std::filesystem::path& path);

}  // namespace botf::utils
