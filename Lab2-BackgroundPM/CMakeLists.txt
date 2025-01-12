cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

set(CMAKE_C_COMPILER "gcc")

add_executable(helloworld source/helloworld.c)
