#include "utils/path.hpp"

#include <cstdlib>
#include <string>

namespace {

std::filesystem::path& contentRootOverride() {
	static std::filesystem::path overridePath;
	return overridePath;
}

}  // namespace

void botf::utils::setContentRootOverride(std::filesystem::path root) {
	contentRootOverride() = std::move(root);
}

std::filesystem::path botf::utils::contentRoot() {
	if (!contentRootOverride().empty()) {
		return contentRootOverride();
	}
	if (const char* env = std::getenv("BOTF_CONTENT_ROOT"); env != nullptr && env[0] != '\0') {
		return std::filesystem::path{env};
	}
#ifdef BOTF_DEFAULT_CONTENT_ROOT
	return std::filesystem::path{BOTF_DEFAULT_CONTENT_ROOT};
#else
	return std::filesystem::current_path() / "content";
#endif
}

std::filesystem::path botf::utils::mapDir(const std::filesystem::path& root,
										  const std::uint32_t mapId) {
	return root / "maps" / std::to_string(mapId);
}

std::filesystem::path botf::utils::mapJsonPath(const std::filesystem::path& root,
											   const std::uint32_t mapId) {
	return mapDir(root, mapId) / "map.json";
}
