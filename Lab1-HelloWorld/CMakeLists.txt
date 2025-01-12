cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_STANDARD 17)

add_executable(helloworld source/helloworld.cpp)
