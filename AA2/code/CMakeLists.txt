cmake_minimum_required(VERSION 3.28)
project(AA2)

set(CMAKE_CXX_STANDARD 20)

add_executable(main src/main.c
        inc/algorithms.h
        src/algorithms.c
        src/measures.c
        inc/measures.h)
add_executable(arduino src/for_arduino.c
)
include_directories(AA2 PRIVATE inc)