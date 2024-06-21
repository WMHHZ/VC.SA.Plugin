cmake_minimum_required(VERSION 3.28)

project(wm_vcchs)

add_library(wm_vcchs SHARED
    FontPatch.cpp
    Main.cpp
)

target_compile_definitions(wm_vcchs PRIVATE)

target_link_libraries(wm_vcchs PRIVATE plugin_vc_lib)

set_target_properties(wm_vcchs PROPERTIES SUFFIX ".asi")
