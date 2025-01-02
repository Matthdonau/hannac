# hannac
hannac is a compiler/interpreter for the programming language hanna.

## hanna
- Methods don't have type signatures. The return type as well as the types of its arguments is
deduced at compile time.
- Methods are only compiled the first time they are being executed. Subsequent calls will use the already
compiled version of it.
- Since methods don't have type signatures, multiple versions of the same method may exist at the same time
 with different type signatures.

 ## Build instructions
    mkdir build && cd build
    cmake  -DCMAKE_BUILD_TYPE=Release ../
    
    # Build the library and tests.
    cmake --build .

    # Run tests
    ../hannac_tests/hannac_tests

    # Run hannac
    ../hannac_compiler <YOUR_HANNA_PROGRAMM>.hanna

