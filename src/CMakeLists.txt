project(FZChessPlusPlus)

cmake_minimum_required(VERSION 2.8)

add_executable(fzchess++ main.cc Chess.cc)

set (CMAKE_CXX_FLAGS "-std=c++11 -O2 -pthread")

FIND_PACKAGE(Threads)
TARGET_LINK_LIBRARIES(fzchess++ ${CMAKE_THREAD_LIBS_INIT})
