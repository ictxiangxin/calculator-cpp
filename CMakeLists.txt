cmake_minimum_required(VERSION 3.10)
project(calculator)

set(CMAKE_CXX_STANDARD 11)

add_executable(calculator main.cpp)

target_link_libraries(calculator mpfr)
