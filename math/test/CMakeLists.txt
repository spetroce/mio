cmake_minimum_required(VERSION 2.8.11)
project(MathTest)

set(MIO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../..")

include("${MIO_INCLUDE_DIR}/mio/cmake/DefaultConfigTypes.cmake")

## mio
include_directories(${MIO_INCLUDE_DIR})

add_executable(ransac_line_test ${MIO_INCLUDE_DIR}/mio/math/test/ransac_line_test.cpp)

