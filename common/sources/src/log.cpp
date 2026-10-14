#include "common/log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>
#include <mutex>

void botf::log::init() {
	static std::once_flag once;
	std::call_once(once, []() {
		auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto logger = std::make_shared<spdlog::logger>("botf", sink);
		logger->set_level(spdlog::level::info);
		logger->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
		spdlog::set_default_logger(std::move(logger));
		spdlog::set_level(spdlog::level::info);
	});
}
