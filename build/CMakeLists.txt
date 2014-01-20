cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

set(app swnt)
set(sd ${CMAKE_CURRENT_LIST_DIR}/../src/)
set(shared ${CMAKE_CURRENT_LIST_DIR}/../shared/)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app "${app}_debug")
endif()

add_definitions(
  -DTRILIBRARY
  -DSINGLE
)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../src
  ${CMAKE_CURRENT_LIST_DIR}/../include
  ${extern_include_dir}
  ${CMAKE_CURRENT_LIST_DIR}/../shared/tinylib/src
  ${shared}/triangle/
  )

if(NOT WIN32)
  set(app_sources
    ${sd}/main.cpp
    ${sd}/swnt/Swnt.cpp
    ${sd}/swnt/GUI.cpp
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
    ${sd}/swnt/Tides.cpp
    ${sd}/swnt/Effects.cpp
    ${sd}/swnt/effects/BaseEffect.cpp
    ${sd}/swnt/effects/Eddy.cpp
    ${sd}/swnt/effects/Splashes.cpp
    ${sd}/swnt/Scene.cpp
    ${sd}/swnt/RGBShift.cpp
    ${sd}/swnt/Weather.cpp
    ${sd}/swnt/Audio.cpp
    ${sd}/swnt/WaterBall.cpp
    ${sd}/swnt/Triangulate.cpp
    ${shared}/triangle/triangle.c
    )
else()
  set(app_sources
    ${sd}/main.cpp
    ${sd}/swnt/Swnt.cpp
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
    ${sd}/swnt/Tides.cpp
    ${sd}/swnt/Effects.cpp
    ${sd}/swnt/effects/BaseEffect.cpp
    ${sd}/swnt/effects/Eddy.cpp
    ${sd}/swnt/effects/Splashes.cpp
    ${sd}/swnt/Scene.cpp
    ${sd}/swnt/RGBShift.cpp
    ${sd}/swnt/Weather.cpp
    ${sd}/swnt/WaterBall.cpp
    ${extern_source_dir}/GLXW/glxw.c
    )
endif()

if(UNIX AND NOT APPLE)
  list(APPEND app_sources ${extern_source_dir}/GLXW/glxw.c)
  message(STATUS "Adding glxw.c for GL-function loading.")
  message(${app_sources})
endif()

if(APPLE)

  find_library(fr_corefoundation CoreFoundation)
  find_library(fr_cocoa Cocoa)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_corevideo CoreVideo)
  find_library(fr_opencl OpenCL)

  set(app_libs
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
    ${extern_lib_dir}/libfmodex.dylib
    ${fr_corefoundation}
    ${fr_cocoa}
    ${fr_opengl}
    ${fr_iokit}  
    ${fr_corevideo}
    ${fr_opencl}
    -lz
    )
elseif(UNIX)
  set(app_libs
    ${extern_lib_dir}/libglfw3.a
    ${extern_lib_dir}/libuv.a
    ${extern_lib_dir}/libfreenect.a
    ${extern_lib_dir}/libusb-1.0.a
    ${extern_lib_dir}/libpng.a
    ${extern_lib_dir}/libopencv_video.a
    ${extern_lib_dir}/libopencv_imgproc.a
    ${extern_lib_dir}/libopencv_core.a
    ${extern_lib_dir}/libz.a
    ${extern_lib_dir}/libAntTweakBar.a
    ${extern_lib_dir}/libcurl.a
    ${extern_lib_dir}/libssl.a
    ${extern_lib_dir}/libcrypto.a
    ${extern_lib_dir}/libcurl.a
    ${extern_lib_dir}/libfmodex64-4.44.30.so
    GL
    X11
    Xxf86vm
    Xrandr
    pthread
    dl
    Xi
    )
  #set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/bin)
  set(CMAKE_INSTALL_RPATH "$ORIGIN")  # look for libraries in the same directory as the executable
elseif(WIN32)

  set(app_libs
     ${extern_lib_dir}/libpng16.lib
     ${extern_lib_dir}/glfw3.lib
     ${extern_lib_dir}/opencv_core300.lib
     ${extern_lib_dir}/opencv_video300.lib
     ${extern_lib_dir}/opencv_cudaarithm300.lib
     ${extern_lib_dir}/opencv_cudabgsegm300.lib
     ${extern_lib_dir}/opencv_cudafilters300.lib
     ${extern_lib_dir}/opencv_cudawarping300.lib
     ${extern_lib_dir}/opencv_imgproc300.lib
     ${extern_lib_dir}/opencv_optim300.lib
     ${extern_lib_dir}/opencv_superres300.lib
     ${extern_lib_dir}/libuv.lib
     ${extern_lib_dir}/libcurl.lib
     Opengl32.lib
     ws2_32.lib
     psapi.lib
     iphlpapi.lib
     )
  add_definitions(
    -DWIN32_LEAN_AND_MEAN
    )
endif()

add_executable(${app} ${app_sources})

target_link_libraries(${app} ${app_libs})

install(TARGETS ${app} DESTINATION bin)

if(APPLE)
  install(FILES ${extern_lib_dir}/libfmodex.dylib DESTINATION bin)
elseif(UNIX)
  install(FILES ${extern_lib_dir}/libfmodex64-4.44.30.so DESTINATION bin)
endif()

