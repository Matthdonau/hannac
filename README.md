# mad_lib_template
## Intention
Template for library dev. Please do not worry about correctnes of provided dummy functions. They are only there as placeholder.

 ## Build instructions
    mkdir build && cd build
    cmake  -DCMAKE_BUILD_TYPE=Release ../
    
    # Build the library, tests and benchmarks
    cmake --build .

    # Run tests
    mad_lib_template_tests/mad_lib_template_tests

    # Run benchmarks
    mad_lib_template_benchmarks/mad_lib_template_benchmarks

