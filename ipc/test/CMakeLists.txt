cmake_minimum_required(VERSION 2.8.11)
project(Misc)

## User defined library/include paths
include(PkgConfigPath.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${MIO_INCLUDE_DIR}/mio/cmake/Modules")

## Setup Release and Debug variables
include(${MIO_INCLUDE_DIR}/mio/cmake/DefaultConfigTypes.cmake)

## mio
include_directories(${MIO_INCLUDE_DIR})

add_executable(shmem_test shmemTest.cpp)
target_link_libraries(shmem_test pthread) #pthread needed for semaphore functions

