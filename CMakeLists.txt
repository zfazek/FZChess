project(FZChess)

cmake_minimum_required(VERSION 2.8)

set (CMAKE_CXX_FLAGS "-Wall")
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
        -Werror
        -std=c++11
        -g
        -O2
        -pthread
        -funroll-loops
        -std=c++11")

    FIND_PACKAGE(Threads)
endif()


add_subdirectory(src)
