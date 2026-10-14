# Format only BOTF project C++ sources: *.cpp / *.h / *.hpp
# under client/, server/, net/, common/sources/, tests/.
#
# Never touches: LIB (SFML/protobuf/googletest), .proto, generated *.pb.cc/h, cmake/.
#
#   BOTF_FORMAT_ON_BUILD=ON  (default) — format, then compile
#   BOTF_FORMAT_ON_BUILD=OFF — skip (manual: cmake --build ... --target botf_format)

option(BOTF_FORMAT_ON_BUILD "Run clang-format before building BOTF targets" ON)

find_program(BOTF_CLANG_FORMAT_EXE
    NAMES clang-format clang-format-18 clang-format-17 clang-format-16
    DOC "clang-format executable for BOTF"
)

set(_botf_format_roots
    "${CMAKE_SOURCE_DIR}/client"
    "${CMAKE_SOURCE_DIR}/server"
    "${CMAKE_SOURCE_DIR}/net"
    "${CMAKE_SOURCE_DIR}/protocol"
    "${CMAKE_SOURCE_DIR}/common/sources"
    "${CMAKE_SOURCE_DIR}/tests"
)

set(BOTF_FORMAT_SOURCES)
foreach(_root IN LISTS _botf_format_roots)
    if(NOT IS_DIRECTORY "${_root}")
        continue()
    endif()
    file(GLOB_RECURSE _botf_chunk CONFIGURE_DEPENDS
        "${_root}/*.cpp"
        "${_root}/*.h"
        "${_root}/*.hpp"
    )
    list(APPEND BOTF_FORMAT_SOURCES ${_botf_chunk})
endforeach()

# Safety filters (generated / junk if ever appear under source roots).
list(FILTER BOTF_FORMAT_SOURCES EXCLUDE REGEX "\\.pb\\.(cc|h)$")
list(FILTER BOTF_FORMAT_SOURCES EXCLUDE REGEX "/build/")
list(FILTER BOTF_FORMAT_SOURCES EXCLUDE REGEX "/_deps/")
list(REMOVE_DUPLICATES BOTF_FORMAT_SOURCES)

if(NOT BOTF_CLANG_FORMAT_EXE)
    message(STATUS "[botf] clang-format not found — format-on-build disabled (install LLVM / clang-format)")
    return()
endif()

if(NOT BOTF_FORMAT_SOURCES)
    message(STATUS "[botf] clang-format: no project *.cpp/*.h/*.hpp matched")
    return()
endif()

list(LENGTH BOTF_FORMAT_SOURCES _botf_format_count)
message(STATUS "[botf] clang-format: ${BOTF_CLANG_FORMAT_EXE}")
message(STATUS "[botf] format-on-build: ${BOTF_FORMAT_ON_BUILD} (${_botf_format_count} project C++ files)")

add_custom_target(botf_format
    COMMAND "${BOTF_CLANG_FORMAT_EXE}" -i ${BOTF_FORMAT_SOURCES}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "[botf] clang-format project C++ (${_botf_format_count} files)"
    VERBATIM
)

if(BOTF_FORMAT_ON_BUILD)
    foreach(_botf_tgt IN ITEMS common logic app net protocol server_engine botf_server botf_client tests)
        if(TARGET ${_botf_tgt})
            add_dependencies(${_botf_tgt} botf_format)
        endif()
    endforeach()
endif()
