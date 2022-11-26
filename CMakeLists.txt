cmake_minimum_required(VERSION 3.12)

project ("ELF")

set(CMAKE_C_STANDARD 90) # C89
add_library(elfc elfc/elf.c)
target_include_directories(elfc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/elfc)

set(CMAKE_CXX_STANDARD 11)
add_library(elfcpp elfcpp/elf.cc)
target_include_directories(elfcpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/elfcpp)
