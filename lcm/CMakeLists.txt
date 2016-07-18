set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_DEBUG "-g")
#message(STATUS "lcmTypes C flags (Release):${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
#message(STATUS "lcmTypes C flags (Debug):${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")

file(GLOB LCM_TYPES_SRC *.c)
add_library(mio_lcmtypes ${LCM_TYPES_SRC})
target_link_libraries(mio_lcmtypes ${LCM_LIBRARIES})

