# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Declares and names the project.

project("face-recognition")

set(OpenCV_STATIC on)
set(OpenCV_DIR "<OPENCV_PATH>/opencv-4.5.4-android-sdk/OpenCV-android-sdk/sdk/native/jni")
find_package(OpenCV REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE on)

# build native_app_glue as a static lib
include_directories(${ANDROID_NDK}/sources/android/native_app_glue ${COMMON_SOURCE_DIR})

include_directories(<SNPE_PATH>/snpe-1.51.0.2663/include/zdl)
add_library(app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")


add_library( # Sets the name of the library.
        facerecognition

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp
        Input_Image.cpp
        FaceRecApp.cpp
        Camera_Input.cpp
        snpe.cpp

        )


find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)



add_library(SNPE SHARED IMPORTED )
set_target_properties(SNPE PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/armeabi-v7a/libSNPE.so)

add_library(CPP SHARED IMPORTED )
set_target_properties(CPP PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/armeabi-v7a/libc++_shared.so)

add_library(CALC SHARED IMPORTED )
set_target_properties(CALC PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/armeabi-v7a/libcalculator.so)

add_library(CALC_HTP SHARED IMPORTED )
set_target_properties(CALC_HTP PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/armeabi-v7a/libcalculator_htp.so)


add_library(PlatformValidatorShared SHARED IMPORTED )
set_target_properties(PlatformValidatorShared PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/armeabi-v7a/libPlatformValidatorShared.so)

add_library(DSP_V2 SHARED IMPORTED )
set_target_properties(DSP_V2 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/armeabi-v7a/libsnpe_dsp_domains_v2.so)

add_library(DSP_V3 SHARED IMPORTED )
set_target_properties(DSP_V3 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/armeabi-v7a/libsnpe_dsp_domains_v3.so)





target_link_libraries( # Specifies the target library.
        facerecognition
        SNPE
        CPP
        CALC
        CALC_HTP
        PlatformValidatorShared
        DSP_V2
        DSP_V3
        camera2ndk
        mediandk
        android
        log
        m

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${OpenCV_LIBS}
        )
