# CMakeLists.txt for project 2 of CS{4/6}290 and ECE{4/6}100 Spring 2021

cmake_minimum_required (VERSION 3.1) # Earlier versions won't support this file

# C project
project(procsim
        DESCRIPTION "OoO Processor Simulator"
        LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(SRC_FILES "${CMAKE_SOURCE_DIR}/src/procsim_driver.cpp"
        "${CMAKE_SOURCE_DIR}/src/procsim.cpp"
        "${CMAKE_SOURCE_DIR}/src/procsim.h"
   )

# Sets required flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set flags for stricter warnings and errors
if( "${CMAKE_CXX_COMPILER_ID}" MATCHES GNU )
    message(STATUS "Using gcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wpointer-arith -Wsign-conversion -Wconversion -pedantic-errors")
elseif( "${CMAKE_CXXCOMPILER_ID}" MATCHES Clang )
    message(STATUS "Using clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wpointer-arith -Wsign-conversion -Wconversion -pedantic-errors")
endif()

# Enable debugging using gdb or lldb depending on operating system
set(CMAKE_BUILD_TYPE Release)

# Generate executable
add_executable(procsim ${SRC_FILES})