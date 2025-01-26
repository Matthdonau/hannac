cmake_minimum_required(VERSION 3.10)

# Generate project
project(hannac_lib C CXX)

# Some settings
set (CMAKE_CXX_STANDARD 17)
set (CXX_STANDARD_REQUIRED ON)

# Neccessary llvm libraries.
find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(llvm_libs 
                                Analysis
                                Core
                                ExecutionEngine
                                InstCombine
                                Object
                                OrcJIT
                                Passes
                                RuntimeDyld
                                ScalarOpts
                                Support
                                native
)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM include dir in: ${LLVM_INCLUDE_DIR}")
message(STATUS "LLVM libs used: ${llvm_libs}")
message(STATUS "LLVM Version: ${LLVM_VERSION}")
if(${LLVM_VERSION} LESS 19.1)
    message(FATAL_ERROR "Unsupported LLVM version found. Minimum needed 19.1")
endif()

# Source files
set(hannac_HEADERS
    "include/FileParser.hpp"
    "include/Lexer.hpp"
    "include/AST.hpp"
    "include/TokenParser.hpp"
    "include/Codegen.hpp"
    "include/JIT.hpp"
    "include/Executor.hpp"
)

# Generate a library as well
add_library(hannac_lib SHARED ${hannac_HEADERS}) 
target_link_libraries(hannac_lib ${llvm_libs} "-ld_classic")
set_target_properties(hannac_lib PROPERTIES LINKER_LANGUAGE CXX)
# Set includes
target_include_directories(hannac_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${LLVM_INCLUDE_DIR}) 
