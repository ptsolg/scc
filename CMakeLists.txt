cmake_minimum_required(VERSION 3.4.3)

include("cmake/config.cmake")
include("cmake/test.cmake")
include("cmake/module.cmake")
include("cmake/wrap.cmake")

project(scc)

add_subdirectory(lib)
add_subdirectory(scc)
