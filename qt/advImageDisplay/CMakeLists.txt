cmake_minimum_required(VERSION 2.8.11)
project(AdvImageDisplay)

option(BUILD_ADV_IMG_DISP_TEST "Build advImageDisplay test executable" OFF)
option(BUILD_ADV_IMG_DISP_SELF "Enabling tells cmake to look for dependencies" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(BUILD_ADV_IMG_DISP_SELF)
  ## User defined library/include paths
  include(PkgConfigPath.cmake)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${MIO_INCLUDE_DIR}/mio/cmake/Modules")

  ## Setup Release and Debug variables
  include(${MIO_INCLUDE_DIR}/mio/cmake/DefaultConfigTypes.cmake)

  ## mio
  include_directories(${MIO_INCLUDE_DIR})

  ## OpenGL (OPENGL_FOUND, OPENGL_INCLUDE_DIR, OPENGL_LIBRARIES)
  find_package(OpenGL REQUIRED)
  include_directories( ${OPENGL_INCLUDE_DIR} )

  ## Qt5
  find_package(Qt5 REQUIRED COMPONENTS Widgets)

  ## OpenCV
  find_package(OpenCV REQUIRED)#HINTS /usr/local/src/share)
  include_directories(${OpenCV_INCLUDE_DIRS})
  message(STATUS "Found OpenCV version ${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH}")
  #add_definitions(-DMIO_OPENCV_VERSION_MAJOR=${OpenCV_VERSION_MAJOR})
endif()

add_definitions(-DADV_IMG_DISP_STRIPES_JPEG="${CMAKE_CURRENT_SOURCE_DIR}/stripes.jpg")

add_library(mio_advImageDisplay advImageDisplay.cpp)
target_link_libraries(mio_advImageDisplay Qt5::Widgets Qt5::Core ${OpenCV_LIBS})

if(BUILD_ADV_IMG_DISP_TEST)
  add_executable(adv_img_disp_test test/main.cpp)
  target_link_libraries(adv_img_disp_test mio_advImageDisplay)
endif()

