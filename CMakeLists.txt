project(FZChess)

cmake_minimum_required(VERSION 2.8)

set (CMAKE_CXX_FLAGS "-Wall -Werror -std=c++11 -g -O2 -pthread -funroll-loops")

FIND_PACKAGE(Threads)

add_subdirectory(src)
