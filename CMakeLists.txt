cmake_minimum_required(VERSION 3.23)
project(ArgvnautProject LANGUAGES CXX)

set(LIBRARY_NAME ArgvnautLib)
set(SAMPLE_NAME ArgvnautSample)
set(TEST_NAME ArgvTest)

# Enable folders in IDEs (optional)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# GoogleTest requires at least C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For IDE IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Avoid warnings by using new policy
cmake_policy(SET CMP0135 NEW)

# ------ Argvnaut Library ------
add_library(${LIBRARY_NAME} INTERFACE)
target_include_directories(${LIBRARY_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(${LIBRARY_NAME} INTERFACE cxx_std_20)

# ------ Argvnaut Sample ------
add_executable(${SAMPLE_NAME} src/main.cpp)
target_link_libraries(
    ${SAMPLE_NAME} 
    PRIVATE ${LIBRARY_NAME})

# ------ Test Target ------
option(ARGVNAUT_BUILD_TESTS "Build argvnaut tests" ${PROJECT_IS_TOP_LEVEL})

if(ARGVNAUT_BUILD_TESTS)
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

  enable_testing()

  file (GLOB TESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
  add_executable(${TEST_NAME} ${TESTS}) 

  target_link_libraries(
      ${TEST_NAME}
      GTest::gtest_main
      ${LIBRARY_NAME}
    )

  include(GoogleTest)
  gtest_discover_tests(${TEST_NAME})
endif()