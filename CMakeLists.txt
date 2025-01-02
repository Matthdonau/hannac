cmake_minimum_required(VERSION 3.10)

# Generate project.
project(hannac)

# Some settings.
set (CMAKE_CXX_STANDARD 17)
set (CXX_STANDARD_REQUIRED ON)

# Compile options.
add_compile_options(-Wall -Wextra -Wpedantic -O2 -g3 -Wno-unused-private-field -Wno-unused-parameter)

# Comnpiler library.
add_subdirectory(hannac_lib)
# Library tests.
add_subdirectory(hannac_tests)

#################### EXECUTABLE ####################
set(hannacexec_SOURCE_FILES
    "src/main.cpp"
)
# Compiler executable.
add_executable(hannac_compiler ${hannacexec_SOURCE_FILES})
include_directories(hannac_compiler ${CMAKE_SOURCE_DIR}/hannac_lib/) 
target_link_libraries(hannac_compiler hannac_lib) 
