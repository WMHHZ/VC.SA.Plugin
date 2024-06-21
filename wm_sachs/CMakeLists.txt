cmake_minimum_required(VERSION 3.28)

project(wm_sachs)

find_package(utf8cpp CONFIG REQUIRED)

add_library(wm_sachs SHARED
    Main.cpp
    FontPatch.cpp
)

target_link_libraries(wm_sachs PRIVATE utf8::cpp utf8cpp::utf8cpp)
target_link_libraries(wm_sachs PRIVATE plugin_sa_lib)

set_target_properties(wm_sachs PROPERTIES SUFFIX ".asi")
