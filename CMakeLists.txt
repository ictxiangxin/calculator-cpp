cmake_minimum_required(VERSION 3.10)
project(calculator)

set(CMAKE_CXX_STANDARD 17)

add_executable(calculator main.cpp tokenize.cpp)

target_link_libraries(calculator mpfr)
