project(proto_messages)

# Protobuf codegen for length-prefixed game packets over SFML TCP.
#
# Resolution order:
#   1) ${BOTF_LIB_ROOT}/protobuf  (Windows / setup.bat layout) — build from source
#   2) find_package(Protobuf)     (system / vcpkg install)
#
# When building from LIB: tests/examples/install OFF so we do not clash with
# LIB/googletest (gmock) and do not require a preinstalled Protobuf package.

set(BOTF_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/messages)

if(NOT BOTF_LIB_ROOT)
    include("${CMAKE_SOURCE_DIR}/cmake/BotfLibRoot.cmake")
endif()

set(BOTF_PROTOBUF_FROM_SOURCE FALSE)

set(_botf_protobuf_src "${BOTF_LIB_ROOT}/protobuf")
if(EXISTS "${_botf_protobuf_src}/CMakeLists.txt")
    message(STATUS "[botf] Using protobuf from ${_botf_protobuf_src}")

    set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable protobuf tests (avoid gmock clash)" FORCE)
    set(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)
    set(protobuf_WITH_ZLIB OFF CACHE BOOL "" FORCE)
    # Match typical VS + SFML dynamic CRT (/MD), not /MT.
    set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)

    add_subdirectory(
        "${_botf_protobuf_src}"
        "${CMAKE_CURRENT_BINARY_DIR}/protobuf_lib"
    )
    set(BOTF_PROTOBUF_FROM_SOURCE TRUE)
else()
    find_package(Protobuf REQUIRED)
endif()

file(GLOB BOTF_PROTO_FILES CONFIGURE_DEPENDS "${BOTF_PROTO_DIR}/*.proto")
if(NOT BOTF_PROTO_FILES)
    message(FATAL_ERROR "[botf] No .proto files under ${BOTF_PROTO_DIR}")
endif()

set(BOTF_PROTO_SRCS)
set(BOTF_PROTO_HDRS)

if(BOTF_PROTOBUF_FROM_SOURCE)
    if(NOT TARGET protoc)
        message(FATAL_ERROR "[botf] LIB protobuf did not create target 'protoc'. Check protobuf_BUILD_PROTOC_BINARIES.")
    endif()

    foreach(_proto IN LISTS BOTF_PROTO_FILES)
        get_filename_component(_abs_proto "${_proto}" ABSOLUTE)
        get_filename_component(_name "${_proto}" NAME_WE)
        set(_src "${CMAKE_CURRENT_BINARY_DIR}/${_name}.pb.cc")
        set(_hdr "${CMAKE_CURRENT_BINARY_DIR}/${_name}.pb.h")

        add_custom_command(
            OUTPUT "${_src}" "${_hdr}"
            COMMAND $<TARGET_FILE:protoc>
                    --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
                    -I ${BOTF_PROTO_DIR}
                    ${_abs_proto}
            DEPENDS "${_abs_proto}" protoc
            COMMENT "[botf] protoc ${_name}.proto"
            VERBATIM
        )
        list(APPEND BOTF_PROTO_SRCS "${_src}")
        list(APPEND BOTF_PROTO_HDRS "${_hdr}")
    endforeach()
else()
    protobuf_generate_cpp(BOTF_PROTO_SRCS BOTF_PROTO_HDRS ${BOTF_PROTO_FILES})
endif()

add_library(${PROJECT_NAME} STATIC ${BOTF_PROTO_SRCS} ${BOTF_PROTO_HDRS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

if(TARGET protobuf::libprotobuf)
    target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf)
elseif(TARGET libprotobuf)
    target_link_libraries(${PROJECT_NAME} PUBLIC libprotobuf)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Protobuf_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${Protobuf_INCLUDE_DIRS})
endif()

if(BOTF_PROTOBUF_FROM_SOURCE)
    message(STATUS "[botf] protobuf: from-source (${_botf_protobuf_src}); protos: ${BOTF_PROTO_FILES}")
else()
    message(STATUS "[botf] protobuf: ${Protobuf_VERSION}; protos: ${BOTF_PROTO_FILES}")
endif()
