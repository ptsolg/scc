cmake_minimum_required(VERSION 3.4.3)

include("cmake/config.cmake")
include("cmake/lib.cmake")

project(scc)

add_subdirectory(lib)
add_subdirectory(include)
add_subdirectory(tools)
