cmake_minimum_required(VERSION 3.10)

# Generate project
project(mad_lib_template_tests)

# Some settings
set (CMAKE_CXX_STANDARD 11)
set (CXX_STANDARD_REQUIRED ON)

# Compile options
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Generate executable
add_executable(mad_lib_template_tests)
set(mad_lib_template_BENCHMARKS_SOURCES
    "basic_tests.cpp"
)
target_sources(mad_lib_template_tests PRIVATE ${mad_lib_template_BENCHMARKS_SOURCES} )

# mad_lib_template
add_dependencies(mad_lib_template_tests mad_lib_template_lib)

# Google benchmark
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
include(FetchContent)
FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(gtest)
message(${CMAKE_SOURCE_DIR}) 
include_directories(mad_lib_template_tests ${gtest_SOURCE_DIR}/include/ ${CMAKE_SOURCE_DIR}/mad_lib_template_lib/)
target_link_libraries(mad_lib_template_tests gtest_main mad_lib_template_lib)