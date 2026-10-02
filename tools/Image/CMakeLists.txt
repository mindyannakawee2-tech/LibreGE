cmake_minimum_required(VERSION 3.20)

project(
    LibreGE_Image
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    libre-image
    src/main.cpp
)

target_include_directories(
    libre-image
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendor/stb
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
        libre-image
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
