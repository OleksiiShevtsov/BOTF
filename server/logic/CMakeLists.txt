project(logic)

# Pure game rules — no SFML, no sockets.

file(GLOB logic_SRC CONFIGURE_DEPENDS
    "include/logic/*.h"
    "include/logic/*.hpp"
    "src/*.cpp"
)

if(logic_SRC)
    add_library(${PROJECT_NAME} STATIC ${logic_SRC})
    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_link_libraries(${PROJECT_NAME} PUBLIC common)
else()
    add_library(${PROJECT_NAME} INTERFACE)
    target_include_directories(${PROJECT_NAME} INTERFACE include)
    target_link_libraries(${PROJECT_NAME} INTERFACE common)
endif()
