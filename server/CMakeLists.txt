# Server-side game rules (not linked by client).
add_subdirectory(logic)
add_subdirectory(engine)

add_executable(botf_server
    main.cpp
)

target_link_libraries(botf_server
    PRIVATE
        server_engine
)
