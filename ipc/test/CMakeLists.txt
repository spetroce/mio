cmake_minimum_required(VERSION 2.8.11)
project(Misc)

## User defined library/include paths
include(PkgConfigPath.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${MIO_INCLUDE_DIR}/mio/cmake/Modules")

## Setup Release and Debug variables
include(${MIO_INCLUDE_DIR}/mio/cmake/DefaultConfigTypes.cmake)

## mio
include_directories(${MIO_INCLUDE_DIR})

add_executable(shared_mem_test shared_mem_test.cpp)
target_link_libraries(shared_mem_test pthread) #pthread needed for semaphore functions

add_executable(shared_mem_test_2 shared_mem_test_2.cpp)
target_link_libraries(shared_mem_test_2 pthread)