cmake_minimum_required(VERSION 3.5)

project(fhetools)

# Needed to deal with const stuff
set(CMAKE_CXX_FLAGS -fpermissive)

# Create a library

# Generate the static library from the library sources
add_library(fhetools STATIC
        src/compile_time_settings.h
        src/debug.h
        src/debug.cpp
        src/tfhe.h
        src/tfhe.cpp
        src/types.h
        src/types.cpp
        )

target_include_directories(fhetools
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )


# Create an executable

# Add an executable with the above sources
add_executable(example_fibonacci
        examples/fibonacci/main.cpp
        )

# link the new hello_library target with the hello_binary target
target_link_libraries(example_fibonacci
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        )
