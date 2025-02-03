cmake_minimum_required(VERSION 3.28)
project(AA)

set(CMAKE_CXX_STANDARD 20)

find_package(CURL)

add_executable(main
        src/main.cpp)

target_link_libraries(main CURL::libcurl)