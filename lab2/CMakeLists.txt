cmake_minimum_required(VERSION 3.16)

project(ColorTransferLab2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_DEBUG_LOG "Enable debug logging" ON)
option(ENABLE_YIQ "Enable YIQ color space helpers" ON)
option(ENABLE_HSL_HSV "Enable HSL/HSV color space helpers" ON)

if(ENABLE_DEBUG_LOG)
  add_compile_definitions(ENABLE_DEBUG_LOG)
endif()

if(ENABLE_YIQ)
  add_compile_definitions(ENABLE_YIQ)
endif()

if(ENABLE_HSL_HSV)
  add_compile_definitions(ENABLE_HSL_HSV)
endif()

add_library(utils STATIC
  src/utils.cpp
)
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_library(colors STATIC
  src/color_spaces.cpp
  src/bmp.cpp
  src/histogram.cpp
  src/color_transfer.cpp
  src/region.cpp
  src/cli.cpp
)
target_include_directories(colors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(colors PUBLIC utils)

add_executable(color_transfer
  src/main.cpp
)
target_include_directories(color_transfer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(color_transfer PRIVATE colors utils)

install(TARGETS color_transfer RUNTIME DESTINATION bin)
