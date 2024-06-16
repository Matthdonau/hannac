cmake_minimum_required(VERSION 3.10)

# Generate project
project(mad_lib_template)

# Some settings
set (CMAKE_CXX_STANDARD 11)
set (CXX_STANDARD_REQUIRED ON)

add_custom_target(mad_lib_template)
add_subdirectory(mad_lib_template_lib)
add_subdirectory(mad_lib_template_benchmarks)
add_subdirectory(mad_lib_template_tests)


add_dependencies(mad_lib_template mad_lib_template_lib mad_lib_template_benchmarks mad_lib_template_tests)
