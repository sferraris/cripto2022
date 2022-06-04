cmake_minimum_required(VERSION 3.22)
project(cripto2022 C)

set(CMAKE_C_STANDARD 11)

add_executable(cripto2022
        stegobmp.c bpmreader.c bpmreader.h)
