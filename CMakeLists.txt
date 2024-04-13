cmake_minimum_required(VERSION 3.28)
project(mymalloc C)

set(CMAKE_C_STANDARD 17)

file (GLOB SOURCES src/*.c)
include_directories(include)
add_library(mymalloc SHARED ${SOURCES})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(mymalloc PRIVATE MYMALLOC_DEBUG)
endif ()

add_executable(malloc_test test/malloc_test.c)
target_link_directories(malloc_test PUBLIC lib)
target_link_libraries(malloc_test mymalloc)