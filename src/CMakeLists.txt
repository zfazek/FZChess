project(FZChessPlusPlus)

cmake_minimum_required(VERSION 2.8)

add_executable(
    fzchess++
    main.cpp
    Chess.cpp
    Hash.cpp
    Util.cpp
    Eval.cpp
    Uci.cpp
    Table.cpp)


