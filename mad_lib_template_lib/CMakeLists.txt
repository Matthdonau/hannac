cmake_minimum_required(VERSION 3.10)

# Generate project
project(mad_lib_template_lib C CXX)

# Some settings
set (CMAKE_CXX_STANDARD 11)
set (CXX_STANDARD_REQUIRED ON)

# Source files
set(mad_lib_template_HEADERS
    "mad_lib_template.hpp"
)

# Compile options
# -g to have symbols for performance profiling
add_compile_options(-Wall -Wextra -Wpedantic -O2 -g3)

# Generate a library as well
add_library(mad_lib_template_lib INTERFACE ${mad_lib_template_HEADERS})
