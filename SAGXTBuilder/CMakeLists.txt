cmake_minimum_required(VERSION 3.28)

project(SAGXTBuilder)

find_package(utf8cpp CONFIG REQUIRED)

add_executable(SAGXTBuilder
    main.cpp
    SAGXT.cpp
)

target_link_libraries(SAGXTBuilder PRIVATE utf8::cpp utf8cpp::utf8cpp)
