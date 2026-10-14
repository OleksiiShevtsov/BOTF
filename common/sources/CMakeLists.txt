project(common)

# Shared headers + botf::log (spdlog color console). No game/net logic here.

file(GLOB_RECURSE COMMON_INCLUDES CONFIGURE_DEPENDS
	"include/*.h"
	"include/*.hpp"
)

file(GLOB COMMON_SRC CONFIGURE_DEPENDS
	"src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${COMMON_SRC} ${COMMON_INCLUDES})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		include
)

# spdlog: prefer LIB/spdlog (setup.bat / setup.sh). System package only as Linux fallback.
if(NOT TARGET spdlog::spdlog)
	if(NOT BOTF_LIB_ROOT)
		include("${CMAKE_SOURCE_DIR}/cmake/BotfLibRoot.cmake")
	endif()

	set(_botf_spdlog_src "${BOTF_LIB_ROOT}/spdlog")
	if(EXISTS "${_botf_spdlog_src}/CMakeLists.txt")
		message(STATUS "[botf] Using spdlog from ${_botf_spdlog_src}")
		set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
		set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
		set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
		set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
		add_subdirectory(
			"${_botf_spdlog_src}"
			"${CMAKE_BINARY_DIR}/_deps/spdlog"
		)
	else()
		find_package(spdlog QUIET)
		if(NOT spdlog_FOUND)
			message(FATAL_ERROR
				"[botf] spdlog not found.\n"
				"  Expected: ${_botf_spdlog_src}\n"
				"  Fix: run setup.bat (or setup.sh) from BattleOfTheFlatland — it clones LIB/spdlog if missing.\n"
				"  Or set BOTF_LIB_ROOT to your LIB folder that contains spdlog/")
		endif()
		message(STATUS "[botf] Using system spdlog")
	endif()
endif()

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		spdlog::spdlog
)
