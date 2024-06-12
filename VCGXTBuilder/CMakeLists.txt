cmake_minimum_required(VERSION 3.28)

project(VCGXTBuilder)

find_package(utf8cpp CONFIG REQUIRED)

add_executable(VCGXTBuilder
    main.cpp
    VCGXT.cpp
)

target_link_libraries(VCGXTBuilder PRIVATE utf8::cpp utf8cpp::utf8cpp)
