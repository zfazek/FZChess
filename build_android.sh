#!/bin/bash

/home/zfazek/android-ndk-r19/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang++ src/*.cpp -O3 -funroll-loops -pie -static-libstdc++ -s -o fzchess

# sudo cp fzchess /var/www/html/
