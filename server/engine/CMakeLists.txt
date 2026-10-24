project(server_engine)

add_library(${PROJECT_NAME} STATIC
    src/game_session.cpp
    src/game_sessions_pool.cpp
    src/server_engine.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        logic
        net
        common
        protocol
)
