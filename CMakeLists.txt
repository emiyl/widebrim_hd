cmake_minimum_required(VERSION 3.20)
project(widebrim C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

set(INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src ${SDL3_INCLUDE_DIRS})

include_directories(${INCLUDE_DIRS})

file(GLOB WIDEBRIM_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(widebrim ${WIDEBRIM_SOURCES})
target_include_directories(widebrim PRIVATE ${INCLUDE_DIRS})
target_link_libraries(widebrim PRIVATE ${SDL3_LINK_LIBRARIES})

# Work around pkg-config library ordering on some platforms.
if(APPLE)
    target_link_options(widebrim PRIVATE -Wl,-framework,CoreFoundation)
endif()
