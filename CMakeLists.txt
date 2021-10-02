cmake_minimum_required(VERSION 3.14)

project("Cpp Dyn"
        VERSION 1.0.0
        LANGUAGES CXX)

option(DYN_BUILD_TESTS "Enable DYN tests" OFF)

add_subdirectory(src/)

if(DYN_BUILD_TESTS)
    message("Build tests")
    add_subdirectory(Tests/)
endif()
