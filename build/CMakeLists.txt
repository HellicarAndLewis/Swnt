cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

set(app swnt)
set(sd ${CMAKE_CURRENT_LIST_DIR}/../src/)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app "${app}_debug")
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../src
  ${CMAKE_CURRENT_LIST_DIR}/../include
  ${extern_include_dir}
  ${CMAKE_CURRENT_LIST_DIR}/../shared/tinylib/src
)

set(app_sources
  ${sd}/main.cpp
  ${sd}/swnt/Swnt.cpp
  ${sd}/swnt/Kinect.cpp
  ${sd}/swnt/Settings.cpp
  ${sd}/swnt/Graphics.cpp
  ${sd}/swnt/Mask.cpp
  ${sd}/swnt/Blur.cpp
  ${sd}/swnt/Threshold.cpp
  ${sd}/swnt/Tracking.cpp
  ${sd}/swnt/Spirals.cpp
  ${sd}/swnt/Flow.cpp
  ${sd}/swnt/Particles.cpp
  ${sd}/swnt/Water.cpp
  ${sd}/swnt/HeightField.cpp
  ${sd}/swnt/Effects.cpp
  ${sd}/swnt/Scene.cpp
  ${sd}/swnt/GUI.cpp
  ${sd}/swnt/RGBShift.cpp
  ${sd}/swnt/Weather.cpp
  ${sd}/ocean/WaterGraphics.cpp
  ${sd}/ocean/Ocean.cpp
)

if(UNIX AND NOT APPLE)
  list(APPEND app_sources ${extern_source_dir}/GLXW/glxw.c)
  message(STATUS "Adding glxw.c for GL-function loading.")
  message(${app_sources})
endif()

add_executable(${app} ${app_sources})

if(APPLE)

  find_library(fr_corefoundation CoreFoundation)
  find_library(fr_cocoa Cocoa)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_corevideo CoreVideo)
  find_library(fr_opencl OpenCL)

  target_link_libraries(
    ${app}
    ${extern_lib_dir}/libglfw3.a
    ${extern_lib_dir}/libuv.a
    ${extern_lib_dir}/libfreenect.a
    ${extern_lib_dir}/libusb-1.0.a
    ${extern_lib_dir}/libpng.a
    ${extern_lib_dir}/libopencv_core.a
    ${extern_lib_dir}/libopencv_imgproc.a
    ${extern_lib_dir}/libopencv_video.a
    ${extern_lib_dir}/libAntTweakBar.a
    ${extern_lib_dir}/libcurl.a
    ${extern_lib_dir}/libcrypto.a
    ${extern_lib_dir}/libssl.a
    ${fr_corefoundation}
    ${fr_cocoa}
    ${fr_opengl}
    ${fr_iokit}  
    ${fr_corevideo}
    ${fr_opencl}
    -lz
    )
elseif(UNIX)
  target_link_libraries(
    ${app}
    ${extern_lib_dir}/libglfw3.a
    ${extern_lib_dir}/libuv.a
    ${extern_lib_dir}/libfreenect.a
    ${extern_lib_dir}/libusb-1.0.a
    ${extern_lib_dir}/libpng.a
    ${extern_lib_dir}/libopencv_video.a
    ${extern_lib_dir}/libopencv_imgproc.a
    ${extern_lib_dir}/libopencv_core.a
    ${extern_lib_dir}/libz.a
    GL
    X11
    Xxf86vm
    Xrandr
    pthread
    dl
    Xi
    )
endif()

install(TARGETS ${app} DESTINATION bin)

