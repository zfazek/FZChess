project(FZChessPlusPlus)

cmake_minimum_required(VERSION 2.8)
subdirs(src)

set (CMAKE_CXX_FLAGS "-Wall -Werror -std=c++11 -g -O2 -pthread -funroll-loops")

FIND_PACKAGE(Threads)
#TARGET_LINK_LIBRARIES(fzchess++ ${CMAKE_THREAD_LIBS_INIT})
