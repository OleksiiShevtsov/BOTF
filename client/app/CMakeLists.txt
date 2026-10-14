project(app)

# Client UI + ClientApp runtime (window, input, scene, TcpClient loop).

file(GLOB app_SRC CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${app_SRC})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        common
        net
        protocol
        sfml-graphics
        sfml-window
        sfml-system
)
