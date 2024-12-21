cmake_minimum_required(VERSION 3.10)

# Generate project
project(hannac)

# Some settings
set (CMAKE_CXX_STANDARD 17)
set (CXX_STANDARD_REQUIRED ON)

add_custom_target(hannac)
add_subdirectory(hannac_lib)
add_subdirectory(hannac_tests)


add_dependencies(hannac hannac_lib hannac_tests)
