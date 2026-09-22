cmake_minimum_required(VERSION 3.20)
project(widebrim C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${SDL3_INCLUDE_DIRS})

include_directories(${INCLUDE_DIRS})

file(GLOB WIDEBRIM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set(WINDOW "sdl3" CACHE STRING "Window backend to use (e.g., sdl3)")
set_property(CACHE WINDOW PROPERTY STRINGS "sdl3")

if(WINDOW STREQUAL "sdl3")
    list(APPEND WIDEBRIM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/window/sdl.c")
else()
    message(FATAL_ERROR "Unsupported window backend: ${WINDOW}. Supported values: sdl3")
endif()

set(RENDERER "sdl3" CACHE STRING "Renderer backend to use (e.g., sdl3)")
set_property(CACHE RENDERER PROPERTY STRINGS "sdl3")

if (RENDERER STREQUAL "sdl3")
    list(APPEND WIDEBRIM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/sdl.c")
else()
    message(FATAL_ERROR "Unsupported renderer backend: ${RENDERER}. Supported values: sdl3")
endif()

set(INPUT "sdl3" CACHE STRING "Input backend to use (e.g., sdl3)")
set_property(CACHE INPUT PROPERTY STRINGS "sdl3")

if (INPUT STREQUAL "sdl3")
    list(APPEND WIDEBRIM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/input/sdl.c")
else()
    message(FATAL_ERROR "Unsupported input backend: ${INPUT}. Supported values: sdl3")
endif()

add_executable(widebrim ${WIDEBRIM_SOURCES})
target_include_directories(widebrim PRIVATE ${INCLUDE_DIRS})
target_link_libraries(widebrim PRIVATE ${SDL3_LINK_LIBRARIES})
target_compile_options(widebrim PRIVATE -pedantic-errors -Wall -Wextra -Werror)

# Work around pkg-config library ordering on some platforms.
if(APPLE)
    target_link_options(widebrim PRIVATE -Wl,-framework,CoreFoundation)
endif()
