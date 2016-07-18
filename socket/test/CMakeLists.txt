cmake_minimum_required(VERSION 2.8.11)
project(sockets) # This generates the fir_SOURCE_DIR variable

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "CMAKE_BUILD_TYPE was not set. Giving default value: Release")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -pedantic -O2")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -pedantic -g")
set(CMAKE_C_FLAGS_DEBUG "-g")

include_directories(../../../)
add_executable(tcp_client tcp_client.cpp)
add_executable(tcp_server tcp_server.cpp)
add_executable(udp_client udp_client.cpp)
add_executable(udp_server udp_server.cpp)

