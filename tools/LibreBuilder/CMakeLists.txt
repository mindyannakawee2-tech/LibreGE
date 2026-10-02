cmake_minimum_required(VERSION 3.20)

project(
    LibreBuilder
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    lbbe
    src/main.cpp
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        lbbe
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
