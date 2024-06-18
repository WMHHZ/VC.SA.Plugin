cmake_minimum_required(VERSION 3.28)

project(wm_vcchs)

add_library(wm_vcchs SHARED
    CCharTable.cpp
    CFontPatch.cpp
    dllmain.cpp
    WMVC.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/plugin_vc/game_vc/RenderWare.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/plugin_vc/game_vc/CTheScripts.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/plugin_vc/game_vc/CFont.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/plugin_vc/game_vc/CSprite.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/plugin_vc/game_vc/CSprite2d.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/plugin_vc/game_vc/CTxdStore.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/plugin_vc/game_vc/CTimer.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/shared/game/CRGBA.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/shared/GameVersion.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/shared/PluginBase.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/shared/DynAddress.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/shared/Patch.cpp
)

target_compile_definitions(wm_vcchs PRIVATE GTAVC GINPUT_COMPILE_VC_VERSION)
target_include_directories(wm_vcchs PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/shared)
target_include_directories(wm_vcchs PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/shared/game)
target_include_directories(wm_vcchs PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/plugin_vc)
target_include_directories(wm_vcchs PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/plugin-sdk/injector)

target_link_libraries(wm_vcchs PRIVATE ginput_patch)

set_target_properties(wm_vcchs PROPERTIES SUFFIX ".asi")
