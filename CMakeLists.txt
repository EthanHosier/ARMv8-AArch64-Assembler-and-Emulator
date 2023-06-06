cmake_minimum_required(VERSION 3.25)
project(armv8_18 C)

set(CMAKE_C_STANDARD 99)

add_executable(armv8_18
        src/emulate.c src/system/system.c src/io/io.c)
