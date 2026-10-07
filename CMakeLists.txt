cmake_minimum_required(VERSION 3.20)

project(
    LibreGE
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# SDL
# ============================================================

set(
    SDL_TESTS
    OFF
    CACHE BOOL ""
    FORCE
)

set(
    SDL_EXAMPLES
    OFF
    CACHE BOOL ""
    FORCE
)

add_subdirectory(
    vendor/SDL
    EXCLUDE_FROM_ALL
)

# ============================================================
# LibreGE
# ============================================================

add_executable(
    LibreGE

    src/main.cpp

    src/graphics/ImageLoader.cpp
    src/graphics/Window.cpp
    src/graphics/Texture.cpp
    src/input/Input.cpp
)

target_include_directories(
    LibreGE

    PRIVATE

    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb
)

target_link_libraries(
    LibreGE

    PRIVATE

    SDL3::SDL3
)

target_compile_features(
    LibreGE

    PRIVATE

    cxx_std_20
)

if(
    CMAKE_CXX_COMPILER_ID
    MATCHES
    "GNU|Clang"
)
    target_compile_options(
        LibreGE

        PRIVATE

        -Wall
        -Wextra
        -Wpedantic
    )
endif()
