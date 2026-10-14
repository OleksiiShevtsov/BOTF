add_subdirectory(app)

add_executable(botf_client
    main.cpp
)

target_link_libraries(botf_client
    PRIVATE
        app
)
