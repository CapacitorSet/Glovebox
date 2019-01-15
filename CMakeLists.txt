cmake_minimum_required(VERSION 3.5)

project(fhetools)

# find_package(OpenMP)
# ${OpenMP_CXX_FLAGS}
# -fpermissive: needed to deal with const stuff
set(CMAKE_CXX_FLAGS "-fpermissive -fno-PIC -no-pie -g -fsanitize=address -DPLAINTEXT=0")

# Create a library

# Generate the static library from the library sources
add_library(fhetools STATIC
        include/compile_time_settings.h
        include/gsl_span_custom.h
        include/tfhe.h
        src/tfhe.cpp
        include/types.h
        src/server.cpp
        src/client.cpp
        src/flow-control.cpp
        src/fixed32.cpp
        src/polynomial.cpp
        include/primitives.h
        src/primitives.cpp
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

# Odd/even [not yet implemented]
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

# Linear regression [not yet implemented]
add_executable(example_linreg_client
        examples/linreg/client.cpp
        )
target_link_libraries(example_linreg_client
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        examples_networking
        )
add_executable(example_linreg_server
        examples/linreg/server.cpp
        )
target_link_libraries(example_linreg_server
        PRIVATE
        fhetools
        tfhe-spqlios-fma
        examples_networking
        )

# http://www.kaizou.org/2014/11/gtest-cmake/
############# start
find_package(Threads REQUIRED)
include(ExternalProject)
ExternalProject_Add(
        gtest
        URL https://github.com/google/googletest/archive/release-1.8.1.zip
        URL_MD5 ad6868782b5952b7476a7c1c72d5a714
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        INSTALL_COMMAND ""
)
ExternalProject_Get_Property(gtest source_dir binary_dir)
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
set_target_properties(libgtest PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        )
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gtest)
set_target_properties(libgmock PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/googlemock/libgmock.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        )
include_directories("${source_dir}/googletest/include"
        "${source_dir}/googlemock/include")
############# end
add_executable(fhetools_test tests/main.cpp)
target_link_libraries(fhetools_test libgtest libgmock)
add_test(fhetools_test fhetools_test)
