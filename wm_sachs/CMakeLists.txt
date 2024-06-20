cmake_minimum_required(VERSION 3.28)

project(wm_sachs)

find_package(utf8cpp CONFIG REQUIRED)

add_library(wm_sachs SHARED
    CGInputPatchSA.cpp
    CCharTable.cpp
    CFontPatch.cpp
    CPlugin.cpp
    dllmain.cpp
)

target_compile_definitions(wm_sachs PRIVATE GINPUT_COMPILE_SA_VERSION)

target_link_libraries(wm_sachs PRIVATE utf8::cpp utf8cpp::utf8cpp)
target_link_libraries(wm_sachs PRIVATE plugin_sa_lib)

set_target_properties(wm_sachs PROPERTIES SUFFIX ".asi")
