cmake_minimum_required(VERSION 3.23)
project(ArgvnautProject LANGUAGES CXX)

set(LIBRARY_NAME ArgvnautLib)
set(TEST_NAME ArgvTest)

# Enable folders in IDEs (optional)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# GoogleTest requires at least C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Avoid warnings by using new policy
cmake_policy(SET CMP0135 NEW)

include(FetchContent)

# ------ Fetch Google Test ------
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
  GIT_SHALLOW TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# ------ Argvnaut Library ------
add_library(${LIBRARY_NAME} INTERFACE)
target_include_directories(${LIBRARY_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ------ Test Target ------
file (GLOB TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
add_executable(${TEST_NAME} ${TESTS}) 

target_link_libraries(
    ${TEST_NAME}
    GTest::gtest_main
    ${LIBRARY_NAME}
  )

  target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

  include(GoogleTest)
  gtest_discover_tests(${TEST_NAME})