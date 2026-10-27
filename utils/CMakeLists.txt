project(utils)

# Content I/O (disk / JSON). nlohmann is PRIVATE — callers use utils API only.

if(NOT BOTF_LIB_ROOT)
	include("${CMAKE_SOURCE_DIR}/cmake/BotfLibRoot.cmake")
endif()

set(_botf_nlohmann_src "${BOTF_LIB_ROOT}/nlohmann_json")
if(NOT TARGET nlohmann_json::nlohmann_json)
	if(EXISTS "${_botf_nlohmann_src}/CMakeLists.txt")
		message(STATUS "[botf] Using nlohmann_json from ${_botf_nlohmann_src}")
		set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
		set(JSON_Install OFF CACHE BOOL "" FORCE)
		add_subdirectory(
			"${_botf_nlohmann_src}"
			"${CMAKE_BINARY_DIR}/_deps/nlohmann_json"
		)
	else()
		message(FATAL_ERROR
			"[botf] nlohmann_json not found.\n"
			"  Expected: ${_botf_nlohmann_src}\n"
			"  Fix: run setup.sh / setup.bat (clones LIB/nlohmann_json).")
	endif()
endif()

add_library(${PROJECT_NAME} STATIC
	src/path.cpp
	src/json_file.cpp
	src/map_loader.cpp
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		include
)

target_compile_definitions(${PROJECT_NAME}
	PUBLIC
		BOTF_DEFAULT_CONTENT_ROOT="${CMAKE_SOURCE_DIR}/content"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		common
	PRIVATE
		nlohmann_json::nlohmann_json
)
