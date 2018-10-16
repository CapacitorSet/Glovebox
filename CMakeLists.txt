cmake_minimum_required(VERSION 3.5)

project(fhetools)

# -fpermissive: needed to deal with const stuff
set(CMAKE_CXX_FLAGS "-fpermissive -fno-PIC -no-pie -g -fsanitize=address")

# Create a library

# Generate the static library from the library sources
add_library(fhetools STATIC
        include/compile_time_settings.h
        include/tfhe.h
        src/tfhe.cpp
        include/types.h
        src/server.cpp
        src/client.cpp
        src/universal.cpp
        )

target_include_directories(fhetools
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )

# Create executables

add_executable(keygen
        examples/keygen/main.cpp
        )
target_link_libraries(keygen
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        )

add_library(examples_networking OBJECT
        examples/dyad.c
        examples/dyad.h
        examples/networking.cpp
        examples/networking.h
        )

target_include_directories(examples_networking
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )

# Fibonacci
add_executable(example_fibonacci_client
        examples/fibonacci/client.cpp
        )
target_link_libraries(example_fibonacci_client
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        examples_networking
        )
add_executable(example_fibonacci_server
        examples/fibonacci/server.cpp
        )
target_link_libraries(example_fibonacci_server
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        examples_networking
        )

# Odd/even
add_executable(example_oddeven_client
        examples/oddeven/client.cpp
        )
target_link_libraries(example_oddeven_client
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        examples_networking
        )
add_executable(example_oddeven_server
        examples/oddeven/server.cpp
        )
target_link_libraries(example_oddeven_server
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        examples_networking
        )