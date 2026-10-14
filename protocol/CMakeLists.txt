project(protocol)

# Wire codec + MessageType factory. No SFML — only protobuf + common DTOs.

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        include
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
        common
        proto_messages
)
