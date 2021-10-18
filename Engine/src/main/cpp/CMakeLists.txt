cmake_minimum_required(VERSION 3.4.1)

set(CORE_HEAD ${CMAKE_SOURCE_DIR}/include/core)
set(COMMON_HEAD ${CMAKE_SOURCE_DIR}/include/common)
set(FILTER_HEAD ${CMAKE_SOURCE_DIR}/include/filter)
set(FACE_HEAD ${CMAKE_SOURCE_DIR}/include/face)
set(PNG_HEAD ${CMAKE_SOURCE_DIR}/third_party/include/include_png)
set(ZIP_HEAD ${CMAKE_SOURCE_DIR}/third_party/include/include_zip)
set(OPENCV_HEAD ${CMAKE_SOURCE_DIR}/third_party/include/opencv_include)
set(DLIB_HEAD ${CMAKE_SOURCE_DIR}/third_party/include/include_dlib)
set(STD_HEAD ${CMAKE_SOURCE_DIR}/third_party/include/include_std)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

include_directories(${CORE_HEAD})
include_directories(${COMMON_HEAD})
include_directories(${FILTER_HEAD})
include_directories(${FACE_HEAD})
include_directories(${PNG_HEAD})
include_directories(${ZIP_HEAD})
include_directories(${OPENCV_HEAD})
include_directories(${DLIB_HEAD})
include_directories(${STD_HEAD})

aux_source_directory(${CMAKE_SOURCE_DIR}/impl/common CPP_COMMON)
aux_source_directory(${CMAKE_SOURCE_DIR}/impl/core CPP_CORE)
aux_source_directory(${CMAKE_SOURCE_DIR}/impl/filter CPP_FILTER)
aux_source_directory(${CMAKE_SOURCE_DIR}/impl/face CPP_FACE)

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

add_library(
        opencv_java4
        SHARED
        IMPORTED)

set_target_properties(
        # lib name
        opencv_java4
        # property
        PROPERTIES IMPORTED_LOCATION
        # value
        ${THIRD_PARTY_DIR}/${ANDROID_ABI}/libopencv_java4.so)

add_library(engine
        SHARED
        engine-lib.cpp
        ${CPP_COMMON}
        ${CPP_CORE}
        ${CPP_FILTER}
        ${CPP_FACE}
        ${DLIB_HEAD}/dlib/all/source.cpp )

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
        opencv_java4
        ${lib-log}
        ${lib-EGL}
        ${lib-GLES})