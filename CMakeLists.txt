cmake_minimum_required(VERSION 3.25)
project(armv8_18 C)

set(CMAKE_C_STANDARD 99)

add_executable(emulate
        src/emulate/emulate.c src/emulate/system/system.c src/emulate/io/io.c src/emulate/memory/memory.c)

add_executable(assemble src/assemble/assemble.c src/ArrayList.c src/assemble/parser/parser.c src/BinarySearchTree.c)
