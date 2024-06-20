cmake_minimum_required(VERSION 3.28)

project(wm_vcchs)

add_library(wm_vcchs SHARED
    CCharTable.cpp
    CFontPatch.cpp
    dllmain.cpp
    WMVC.cpp
)

target_compile_definitions(wm_vcchs PRIVATE GINPUT_COMPILE_VC_VERSION)

target_link_libraries(wm_vcchs PRIVATE plugin_vc_lib)

set_target_properties(wm_vcchs PROPERTIES SUFFIX ".asi")
