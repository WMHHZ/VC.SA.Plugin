cmake_minimum_required(VERSION 3.28)

project(ginput_patch)

add_library(ginput_patch INTERFACE
CGInputPatch.cpp
)
