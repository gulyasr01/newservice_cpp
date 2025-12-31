cmake_minimum_required(VERSION 3.22)
project(newservice_cpp LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use C++23 everywhere
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Debug unless the user picks another config
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Header-only "library"
add_library(newsaggregator INTERFACE)
target_include_directories(newsaggregator INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(newservice src/main.cpp)
target_link_libraries(newservice PRIVATE newsaggregator)

# === GoogleTest ===
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0  # recommended stable release
)

# Forces gtest to build as static libs
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# === Add your test executable ===
add_executable(
    test_newservice
    tests/test_newservice.cpp
)

target_link_libraries(
    test_newservice
    PRIVATE
        gtest_main
        newsaggregator
)

# === Register tests with CTest ===
include(GoogleTest)
gtest_discover_tests(test_newservice)