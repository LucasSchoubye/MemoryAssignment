cmake_minimum_required(VERSION 3.21)
project(MemoryAssignment C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(
        memorytests.c
        mymem.c
        mymem.h)
