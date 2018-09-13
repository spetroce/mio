set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "CMAKE_BUILD_TYPE was not set. Giving default value: Release")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE Release)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "-pedantic -O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-pedantic -g  -O0 -DDEBUG -D_DEBUG")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG -D_DEBUG")
# Handle some compile flag options
option(DEBUG_FLAG_GLIBCXX_DEBUG "use gcc flag -D_GLIBCXX_DEBUG when CMAKE_BUILD_TYPE is Debug" OFF)
if(DEBUG_FLAG_GLIBCXX_DEBUG)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
endif()
message(STATUS "C++ flags (Release):${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "C++ flags (Debug):${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C flags (Release):${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "C flags (Debug):${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")

