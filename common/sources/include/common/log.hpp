#pragma once

// Public logging API for BOTF. Call sites include this header only —
// do not #include <spdlog/...> outside common/log.cpp / this wrapper.

#include <spdlog/spdlog.h>

#include <utility>

namespace botf::log {

// Colored stdout sink (stdout_color_mt). Safe to call more than once.
void init();

template <typename... Args>
inline void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
	init();
	spdlog::info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
	init();
	spdlog::warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
	init();
	spdlog::error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void debug(spdlog::format_string_t<Args...> fmt, Args&&... args) {
	init();
	spdlog::debug(fmt, std::forward<Args>(args)...);
}

}  // namespace botf::log
