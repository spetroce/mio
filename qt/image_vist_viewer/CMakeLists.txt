cmake_minimum_required(VERSION 2.8.11)
project(image_list_viewer)

option(BUILD_IMG_LIST_VIEWER_TEST "Build image_list_viewer test executable" OFF)
option(BUILD_IMG_LIST_VIEWER_SELF "Enabling tells cmake to look for dependencies" OFF)

if(BUILD_IMG_LIST_VIEWER_SELF)
  ## User defined library/include paths
  include(PkgConfigPath.cmake)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${MIO_INCLUDE_DIR}/mio/CMakeModules")

  ## Setup Release and Debug variables
  include(${MIO_INCLUDE_DIR}/mio/cmake/DefaultConfigTypes.cmake)

  ## mio
  include_directories(${MIO_INCLUDE_DIR})

  ## OpenGL
  # OPENGL_FOUND, OPENGL_INCLUDE_DIR, OPENGL_LIBRARIES
  find_package(OpenGL REQUIRED)
  include_directories( ${OPENGL_INCLUDE_DIR} )

  ## Qt5
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
  find_package(Qt5 REQUIRED COMPONENTS Widgets)

  ## OpenCV
  find_package(OpenCV REQUIRED)#HINTS /usr/local/src/share)
  include_directories(${OpenCV_INCLUDE_DIRS})
  set(OPENCV_LIBRARIES ${OpenCV_LIBS})
  message(STATUS "Found OpenCV version ${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH}")
  #add_definitions(-DMIO_OPENCV_VERSION_MAJOR=${OpenCV_VERSION_MAJOR})
endif()

## mio_adv_image_display
add_subdirectory(${MIO_INCLUDE_DIR}/mio/qt/adv_image_display ${CMAKE_CURRENT_BINARY_DIR}/mio/qt/adv_image_display)

add_definitions(-DIMG_LIST_VIEWER_EARTH_JPEG_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

qt5_wrap_ui(ui_image_list_viewer image_list_viewer.ui)
add_library(mio_image_list_viewer image_list_viewer.cpp ${ui_image_list_viewer})
target_link_libraries(mio_image_list_viewer Qt5::Widgets ${OPENCV_LIBRARIES} mio_adv_image_display)

if(BUILD_IMG_LIST_VIEWER_TEST)
  add_executable(img_list_viewer_test test/main.cpp)
  target_link_libraries(img_list_viewer_test mio_image_list_viewer)
endif()

