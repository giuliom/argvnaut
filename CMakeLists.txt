cmake_minimum_required(VERSION 3.23)
project(ArgvnautProject LANGUAGES CXX)

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