cmake_minimum_required(VERSION 2.8.11)

option(ADV_SLIDER_WID_FIND_QT5 "Turn on if compiling advSliderWidget alone" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(ADV_SLIDER_WID_FIND_QT5)
  find_package(Qt5 REQUIRED COMPONENTS Widgets)
endif()

add_library(mio_advSliderWidget advSliderWidget.cpp)
target_link_libraries(mio_advSliderWidget Qt5::Widgets)

option(BUILD_ADV_SLIDER_WID_TEST "Build advSliderWidget test executable" OFF)

if(BUILD_ADV_SLIDER_WID_TEST)
  add_executable(adv_slider_test test/advSliderTest.cpp)
  target_link_libraries(adv_slider_test mio_advSliderWidget)

  add_executable(dbl_adv_slider_test test/dblAdvSliderTest.cpp)
  target_link_libraries(dbl_adv_slider_test mio_advSliderWidget)
endif()

