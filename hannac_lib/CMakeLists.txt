cmake_minimum_required(VERSION 3.10)

# Generate project
project(hannac_lib C CXX)

# Some settings
set (CMAKE_CXX_STANDARD 17)
set (CXX_STANDARD_REQUIRED ON)

# Source files
set(hannac_HEADERS
    "include/FileParser.hpp"
    "include/Lexer.hpp"
    "include/AST.hpp"
)

# Compile options
# -g to have symbols for performance profiling
add_compile_options(-Wall -Wextra -Wpedantic -O2 -g3)

# Generate a library as well
add_library(hannac_lib SHARED ${hannac_HEADERS}) 
set_target_properties(hannac_lib PROPERTIES LINKER_LANGUAGE CXX)

# Set includes
target_include_directories(hannac_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) 

#################### EXECUTABLE ####################
set(hannacexec_SOURCE_FILES
    "src/main.cpp"
)
# Add executable
add_executable(hannac_compiler ${hannacexec_SOURCE_FILES})
include_directories(hannac_compiler ${CMAKE_SOURCE_DIR}/hannac_lib/)
target_link_libraries(hannac_compiler hannac_lib)