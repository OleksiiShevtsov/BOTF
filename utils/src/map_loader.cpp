#include "utils/map_loader.hpp"

#include "utils/json_file.hpp"
#include "utils/path.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace {

bool isNumericDirName(const std::string& name) {
	if (name.empty()) {
		return false;
	}
	for (const char c : name) {
		if (!std::isdigit(static_cast<unsigned char>(c))) {
			return false;
		}
	}
	return true;
}

}  // namespace

std::vector<botf::common::ID> botf::utils::listMapIds() {
	std::vector<common::ID> ids;
	const fs::path mapsRoot = contentRoot() / "maps";
	std::error_code ec;
	if (!fs::is_directory(mapsRoot, ec)) {
		return ids;
	}
	for (const auto& entry : fs::directory_iterator(mapsRoot, ec)) {
		if (ec) {
			break;
		}
		if (!entry.is_directory()) {
			continue;
		}
		const std::string name = entry.path().filename().string();
		if (!isNumericDirName(name)) {
			continue;
		}
		ids.push_back(static_cast<common::ID>(std::stoul(name)));
	}
	std::sort(ids.begin(), ids.end());
	return ids;
}

std::optional<botf::utils::MapData> botf::utils::loadMap(const common::ID mapId) {
	const fs::path path = mapJsonPath(contentRoot(), mapId);
	const auto text = readFile(path);
	if (!text) {
		return std::nullopt;
	}

	nlohmann::json j;
	try {
		j = nlohmann::json::parse(*text);
	} catch (const nlohmann::json::parse_error&) {
		return std::nullopt;
	}

	if (!j.is_object() || !j.contains("id") || !j.contains("server")) {
		return std::nullopt;
	}

	MapData data;
	try {
		data.id = j.at("id").get<common::ID>();
		if (data.id != mapId) {
			return std::nullopt;
		}
		if (j.contains("name") && j.at("name").is_string()) {
			data.name = j.at("name").get<std::string>();
		}

		const auto& server = j.at("server");
		data.width = server.at("width").get<float>();
		data.height = server.at("height").get<float>();
		data.maxNumberPlayers = server.at("maxNumberPlayers").get<std::uint32_t>();
		if (data.maxNumberPlayers < 2) {
			return std::nullopt;
		}
		if (!server.contains("spawns") || !server.at("spawns").is_array()) {
			return std::nullopt;
		}
		for (const auto& spawn : server.at("spawns")) {
			if (!spawn.is_array() || spawn.size() != 2) {
				return std::nullopt;
			}
			data.spawns.push_back(common::Point{spawn.at(0).get<float>(), spawn.at(1).get<float>()});
		}
		if (data.spawns.size() != data.maxNumberPlayers) {
			return std::nullopt;
		}

		if (j.contains("client") && j.at("client").is_object()) {
			const auto& client = j.at("client");
			if (client.contains("background") && client.at("background").is_string()) {
				data.clientBackground = client.at("background").get<std::string>();
			}
		}
	} catch (const nlohmann::json::exception&) {
		return std::nullopt;
	}

	return data;
}
