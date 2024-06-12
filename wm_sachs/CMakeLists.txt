cmake_minimum_required(VERSION 3.28)

project(wm_sachs)

find_package(utf8cpp CONFIG REQUIRED)

add_library(wm_sachs SHARED
    CCharTable.cpp
    CFontPatch.cpp
    CPlugin.cpp
    dllmain.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa/game_sa/RenderWare.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa/game_sa/CTheScripts.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa/game_sa/CFont.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa/game_sa/CSprite.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa/game_sa/CSprite2d.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa/game_sa/CTxdStore.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa/game_sa/CRect.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa/game_sa/CVector2D.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/shared/game/CRGBA.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/shared/GameVersion.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/shared/PluginBase.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/shared/DynAddress.cpp
    ${CMAKE_SOURCE_DIR}/plugin-sdk/shared/Patch.cpp
)

target_compile_definitions(wm_sachs PRIVATE GTASA)
target_include_directories(wm_sachs PRIVATE ${CMAKE_SOURCE_DIR}/plugin-sdk/shared)
target_include_directories(wm_sachs PRIVATE ${CMAKE_SOURCE_DIR}/plugin-sdk/shared/game)
target_include_directories(wm_sachs PRIVATE ${CMAKE_SOURCE_DIR}/plugin-sdk/plugin_sa)
target_include_directories(wm_sachs PRIVATE ${CMAKE_SOURCE_DIR}/plugin-sdk/injector)

target_link_libraries(wm_sachs PRIVATE utf8::cpp utf8cpp::utf8cpp)

set_target_properties(wm_sachs PROPERTIES SUFFIX ".asi")
