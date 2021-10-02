add_library(DYN INTERFACE)

add_subdirectory(dyn)

set(CMAKE_CXX_EXTENSIONS false)

#useful for IDEs
set_property(
    TARGET DYN
    PROPERTY INTERFACE_SOURCES)

target_include_directories(DYN INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

target_compile_features(DYN INTERFACE cxx_std_20)

if(MSVC)
     target_compile_options(DYN INTERFACE /Zc:preprocessor /Zc:__cplusplus /permissive- /bigobj)
endif()

add_library(DYN::DYN ALIAS DYN)
