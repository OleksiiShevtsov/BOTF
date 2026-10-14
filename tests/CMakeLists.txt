project(tests)

# Own gtest entrypoint (tests/main.cpp). Do not link gtest_main.
file(GLOB tests_SRC CONFIGURE_DEPENDS
  "*.cpp"
)

file(GLOB tests_HELPER_HEADERS CONFIGURE_DEPENDS
  "helpers/*.h"
  "helpers/*.hpp"
)

if(NOT tests_SRC)
  add_custom_target(${PROJECT_NAME}
    COMMENT "tests: no sources yet (add *.cpp under tests/)"
  )
  return()
endif()

if(NOT TARGET GTest::gtest AND NOT TARGET gtest)
  message(WARNING "[botf] GTest not found — skipping tests executable")
  add_custom_target(${PROJECT_NAME}
    COMMENT "tests: GTest missing (install libgtest-dev or LIB/googletest)"
  )
  return()
endif()

add_executable(${PROJECT_NAME} ${tests_SRC} ${tests_HELPER_HEADERS})

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Prefer imported GTest::gtest (find_package); fall back to plain gtest (LIB subdir).
set(_botf_gtest_lib GTest::gtest)
if(NOT TARGET GTest::gtest)
  set(_botf_gtest_lib gtest)
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${_botf_gtest_lib}
    server_engine
    app
    net
    common
    protocol
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}
  WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  DISCOVERY_TIMEOUT 30
)
