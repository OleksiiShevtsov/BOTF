#include "utils/figure_loader.hpp"

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

std::vector<botf::common::ID> botf::utils::listFigureIds() {
	std::vector<common::ID> ids;
	const fs::path figuresRoot = contentRoot() / "figures";
	std::error_code ec;
	if (!fs::is_directory(figuresRoot, ec)) {
		return ids;
	}
	for (const auto& entry : fs::directory_iterator(figuresRoot, ec)) {
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

std::optional<botf::utils::FigureData> botf::utils::loadFigure(const common::ID figureId) {
	const fs::path path = figureJsonPath(contentRoot(), static_cast<std::uint32_t>(figureId));
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

	FigureData data;
	try {
		data.id = j.at("id").get<common::ID>();
		if (data.id != figureId) {
			return std::nullopt;
		}
		if (j.contains("name") && j.at("name").is_string()) {
			data.name = j.at("name").get<std::string>();
		}

		const auto& server = j.at("server");
		if (!server.contains("hitbox") || !server.at("hitbox").is_object()) {
			return std::nullopt;
		}
		const auto& hitbox = server.at("hitbox");
		data.hitboxWidth = hitbox.at("width").get<float>();
		data.hitboxHeight = hitbox.at("height").get<float>();
		if (data.hitboxWidth <= 0.f || data.hitboxHeight <= 0.f) {
			return std::nullopt;
		}
		if (hitbox.contains("offset")) {
			const auto& offset = hitbox.at("offset");
			if (!offset.is_array() || offset.size() != 2) {
				return std::nullopt;
			}
			data.hitboxOffset =
				common::Point{offset.at(0).get<float>(), offset.at(1).get<float>()};
		}

		data.acceleration = server.at("acceleration").get<float>();
		data.friction = server.at("friction").get<float>();
		if (data.acceleration <= 0.f || data.friction < 0.f) {
			return std::nullopt;
		}
	} catch (const nlohmann::json::exception&) {
		return std::nullopt;
	}

	return data;
}
