cmake_minimum_required(VERSION 3.20)
project(xary VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -O3)
endif()

include_directories(include)

set(SOURCES
    src/main.cpp
    src/core/Stream.cpp
    src/core/StreamWriter.cpp
    src/core/BufferView.cpp
    src/cli/ArgumentParser.cpp
)

add_executable(xary ${SOURCES})
target_include_directories(xary PUBLIC include)

# Apply static linking conditionally based on OS/Compiler
if(WIN32 AND NOT MSVC)
    # MinGW / GCC on Windows
    target_link_options(xary PRIVATE -static -static-libgcc -static-libstdc++)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Linux GCC/Clang
    target_link_options(xary PRIVATE -static-libgcc -static-libstdc++)
endif()