cmake_minimum_required(VERSION 3.28)

project(wmhhz_vcsa)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include("VC-LTL helper for cmake.cmake")

add_subdirectory(SAGXTBuilder)
add_subdirectory(VCGXTBuilder)
add_subdirectory(thirdparty/plugin-sdk-build)
add_subdirectory(wm_vcchs)
add_subdirectory(wm_sachs)
