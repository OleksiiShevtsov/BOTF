#include "utils/json_file.hpp"

#include <fstream>
#include <sstream>

std::optional<std::string> botf::utils::readFile(const std::filesystem::path& path) {
	std::ifstream in{path, std::ios::in | std::ios::binary};
	if (!in) {
		return std::nullopt;
	}
	std::ostringstream ss;
	ss << in.rdbuf();
	if (!in.good() && !in.eof()) {
		return std::nullopt;
	}
	return ss.str();
}
