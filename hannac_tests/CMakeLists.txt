cmake_minimum_required(VERSION 3.10)

# Generate project
project(hannac_tests)

# Some settings
set (CMAKE_CXX_STANDARD 17)
set (CXX_STANDARD_REQUIRED ON)

# Compile options
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Generate executable
add_executable(hannac_tests)
set(hannac_BENCHMARKS_SOURCES
    "FileParser_tests.cpp"
)
target_sources(hannac_tests PRIVATE ${hannac_BENCHMARKS_SOURCES} )

# hannac
add_dependencies(hannac_tests hannac_lib)

# Google benchmark
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
include(FetchContent)
FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0 # googletest seems to have moved on to c++17
)
FetchContent_MakeAvailable(gtest)
message(${CMAKE_SOURCE_DIR}) 
include_directories(hannac_tests ${gtest_SOURCE_DIR}/include/ ${CMAKE_SOURCE_DIR}/hannac_lib/)
target_link_libraries(hannac_tests gtest_main hannac_lib)