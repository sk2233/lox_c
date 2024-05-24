cmake_minimum_required(VERSION 3.26)
project(my_lox C)

set(CMAKE_C_STANDARD 11)

include_directories(
        ${PROJECT_SOURCE_DIR}/src
)

file(GLOB_RECURSE SOURCE_LIST "src/*.c" "src/*.h")

add_executable(my_lox main.c ${SOURCE_LIST}
        src/value.h)
