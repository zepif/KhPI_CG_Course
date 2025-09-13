cmake_minimum_required(VERSION 3.16)

project(Crystali LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

set(SRCS
    src/main.cpp
    src/app.cpp
    src/automaton.cpp
    src/render.cpp
    src/ui.cpp
    src/utils.cpp
)

add_executable(crystali WIN32 ${SRCS})

target_compile_definitions(crystali PRIVATE
    UNICODE _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_compile_options(crystali PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(crystali PRIVATE gdi32 user32 comdlg32)
