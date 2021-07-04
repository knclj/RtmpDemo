# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)
project("rtmpdemo")
add_subdirectory(librtmp)
include_directories(librtmp ${CMAKE_SOURCE_DIR}/libx264/include)

set(libX264Path ${CMAKE_SOURCE_DIR}/libx264/libs/${CMAKE_ANDROID_ARCH_ABI})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${libX264Path}")

add_library( # Sets the name of the library.
             native-lib
             # Sets the library as a shared library.
             SHARED
             # Provides a relative path to your source file(s).
             native-lib.cpp )


find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

target_link_libraries( # Specifies the target library.
                       native-lib
                       rtmp
                        x264
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )