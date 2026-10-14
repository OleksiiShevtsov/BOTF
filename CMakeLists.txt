cmake_minimum_required(VERSION 3.16)
project(BattleOfTheFlatland)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("${CMAKE_SOURCE_DIR}/cmake/BotfLibRoot.cmake")

# Guard: binary dir must not be inside the source tree.
# Use CMakePresets.json → $env{CMAKE_BUILDS_ROOT}/${sourceDirName}/debug|release
get_filename_component(_botf_src "${CMAKE_SOURCE_DIR}" ABSOLUTE)
get_filename_component(_botf_bin "${CMAKE_BINARY_DIR}" ABSOLUTE)
string(FIND "${_botf_bin}" "${_botf_src}/" _botf_bin_inside)
if(_botf_bin_inside EQUAL 0)
    message(WARNING
        "[botf] Build directory is inside the project:\n"
        "  ${CMAKE_BINARY_DIR}\n"
        "Use preset debug/release (CMAKE_BUILDS_ROOT), not an in-tree build/ folder.")
endif()

function(botf_add_lib_subdir dep_name)
    set(_candidates "${BOTF_LIB_ROOT}/${dep_name}")
    if("${dep_name}" STREQUAL "SFML")
        list(APPEND _candidates "${BOTF_LIB_ROOT}/sfml")
        file(GLOB _sfml_ver_dirs LIST_DIRECTORIES true "${BOTF_LIB_ROOT}/SFML-*")
        list(APPEND _candidates ${_sfml_ver_dirs})
    endif()

    foreach(_dir IN LISTS _candidates)
        if(_dir AND EXISTS "${_dir}/CMakeLists.txt")
            message(STATUS "[botf] Using ${dep_name} from ${_dir}")
            add_subdirectory("${_dir}" "${CMAKE_BINARY_DIR}/_deps/${dep_name}")
            return()
        endif()
    endforeach()
    message(WARNING "[botf] ${dep_name} not found under ${BOTF_LIB_ROOT}. Run setup.bat (skips clone if already present).")
endfunction()

if(WIN32)
    botf_add_lib_subdir(SFML)
    botf_add_lib_subdir(googletest)
else()
    find_package(SFML 2.5 COMPONENTS system window graphics network REQUIRED)
    find_package(GTest QUIET)
endif()

enable_testing()

# Shared libraries
add_subdirectory(common/sources ${CMAKE_CURRENT_BINARY_DIR}/common)
add_subdirectory(common/protobuf ${CMAKE_CURRENT_BINARY_DIR}/proto_messages)
add_subdirectory(protocol)
add_subdirectory(net)

# Executables (client owns app/, server owns logic/)
add_subdirectory(server)
add_subdirectory(client)

# tests after server/client so it can link server_engine + app
add_subdirectory(tests)

# clang-format before botf_server / botf_client (see cmake/BotfClangFormat.cmake)
include("${CMAKE_SOURCE_DIR}/cmake/BotfClangFormat.cmake")
