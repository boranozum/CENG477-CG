cmake_minimum_required(VERSION 3.22)
project(HW3)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(HW3
        fat.h
        main.cpp utils.cpp utils.h read.cpp read.h find.cpp find.h create.cpp create.h move.cpp move.h)
