cmake_minimum_required(VERSION 3.22)

project(DriftDTL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE DRIFTDTL_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(driftdtl ${DRIFTDTL_SOURCES})
target_include_directories(driftdtl PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(MSVC)
  target_compile_options(driftdtl PRIVATE /W4 /permissive-)
else()
  target_compile_options(driftdtl PRIVATE -Wall -Wextra -Wpedantic)
endif()

