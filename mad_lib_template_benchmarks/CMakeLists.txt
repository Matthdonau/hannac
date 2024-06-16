cmake_minimum_required(VERSION 3.10)

# Generate project
project(mad_lib_template_benchmarks)

# Some settings
set (CMAKE_CXX_STANDARD 11)
set (CXX_STANDARD_REQUIRED ON)

# Compile options
add_compile_options(-Wall -Wextra -pedantic -Werror)

# Generate executable
add_executable(mad_lib_template_benchmarks)
set(mad_lib_template_BENCHMARKS_SOURCES
    "basic_benchmarks.cpp"
)
target_sources(mad_lib_template_benchmarks PRIVATE ${mad_lib_template_BENCHMARKS_SOURCES} )

# mad_lib_template
add_dependencies(mad_lib_template_benchmarks mad_lib_template_lib)

# Google benchmark
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
include(FetchContent)
FetchContent_Declare(
  google_benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(google_benchmark)
include_directories(mad_lib_template_benchmarks ${google_benchmark_SOURCE_DIR}/include/ ${CMAKE_SOURCE_DIR}/mad_lib_template_lib/)
target_link_libraries(mad_lib_template_benchmarks benchmark::benchmark mad_lib_template_lib)