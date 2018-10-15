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

# Fibonacci
add_executable(example_fibonacci_client
        examples/fibonacci/client.cpp
        examples/networking.cpp
        examples/networking.h
        src/dyad.c
        )
target_link_libraries(example_fibonacci_client
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        )
add_executable(example_fibonacci_server
        examples/fibonacci/server.cpp
        examples/networking.cpp
        examples/networking.h
        src/dyad.c)
target_link_libraries(example_fibonacci_server
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        )
