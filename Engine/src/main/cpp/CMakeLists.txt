cmake_minimum_required(VERSION 3.4.1)

set(CORE_HEAD ${CMAKE_SOURCE_DIR}/include/core)
set(COMMON_HEAD ${CMAKE_SOURCE_DIR}/include/common)
set(FILTER_HEAD ${CMAKE_SOURCE_DIR}/include/filter)

include_directories(${CORE_HEAD})
include_directories(${COMMON_HEAD})
include_directories(${FILTER_HEAD})

aux_source_directory(${CMAKE_SOURCE_DIR}/impl/common CPP_COMMON)
aux_source_directory(${CMAKE_SOURCE_DIR}/impl/core CPP_CORE)
aux_source_directory(${CMAKE_SOURCE_DIR}/impl/filter CPP_FILTER)

add_library(engine
        SHARED
        engine-lib.cpp
        ${CPP_COMMON}
        ${CPP_CORE}
        ${CPP_FILTER})

find_library( # name of the lib
        lib-log
        # name of lib in NDK
        log )

find_library( # name of the lib
        lib-EGL
        # name of lib in NDK
        EGL )

find_library( # name of the lib
        lib-GLES
        # name of lib in NDK
        GLESv2 )

target_link_libraries( # Specifies the target library.
        engine
        # Links the target library to the libraries below
        jnigraphics
        android
        ${lib-log}
        ${lib-EGL}
        ${lib-GLES})