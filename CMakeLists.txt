
cmake_minimum_required(VERSION 3.20)

project(simple-c-compiler)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wextra -Wpedantic -Werror)

add_subdirectory(src)
add_subdirectory(tests)