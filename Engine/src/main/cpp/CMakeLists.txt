cmake_minimum_required(VERSION 3.4.1)

set(CORE_HEAD ${CMAKE_SOURCE_DIR}/include/core)
set(COMMON_HEAD ${CMAKE_SOURCE_DIR}/include/common)
set(FILTER_HEAD ${CMAKE_SOURCE_DIR}/include/filter)
set(PNG_HEAD ${CMAKE_SOURCE_DIR}/third_party/include/include_png)
set(ZIP_HEAD ${CMAKE_SOURCE_DIR}/third_party/include/include_zip)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

include_directories(${CORE_HEAD})
include_directories(${COMMON_HEAD})
include_directories(${FILTER_HEAD})
include_directories(${PNG_HEAD})
include_directories(${ZIP_HEAD})

aux_source_directory(${CMAKE_SOURCE_DIR}/impl/common CPP_COMMON)
aux_source_directory(${CMAKE_SOURCE_DIR}/impl/core CPP_CORE)
aux_source_directory(${CMAKE_SOURCE_DIR}/impl/filter CPP_FILTER)

add_library(
        lib-zip
        SHARED
        IMPORTED)

set_target_properties(
        # lib name
        lib-zip
        # property
        PROPERTIES IMPORTED_LOCATION
        # value
        ${THIRD_PARTY_DIR}/${ANDROID_ABI}/libz.so)

add_library(
        lib-png
        SHARED
        IMPORTED)

set_target_properties(
        # lib name
        lib-png
        # property
        PROPERTIES IMPORTED_LOCATION
        # value
        ${THIRD_PARTY_DIR}/${ANDROID_ABI}/libpng.so)

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
        GLESv3 )

target_link_libraries( # Specifies the target library.
        engine
        # Links the target library to the libraries below
        jnigraphics
        android
        lib-zip
        lib-png
        ${lib-log}
        ${lib-EGL}
        ${lib-GLES})