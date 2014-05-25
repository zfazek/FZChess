add_executable(fzchess++ FZChessPlus.cc UCI.cc)

set (CMAKE_CXX_FLAGS "-std=c++11 -O2")

FIND_PACKAGE(Threads)
TARGET_LINK_LIBRARIES(fzchess++ ${CMAKE_THREAD_LIBS_INIT})
