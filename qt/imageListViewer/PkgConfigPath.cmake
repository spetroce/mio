set(SYSTEM_DETECTED ON)
if(UNIX AND NOT APPLE)
  set(CODE_PREFIX_ "/home/$ENV{USER}")
  find_path(Qt5_GCC_64_CMAKE_DIR "gcc_64/lib/cmake/Qt5/Qt5Config.cmake"
            PATHS "/home/$ENV{USER}/Qt/*" "/home/$ENV{USER}/code/src/Qt/*"
            NO_DEFAULT_PATH)
  if(EXISTS ${Qt5_GCC_64_CMAKE_DIR})
    set(Qt5_DIR "${Qt5_GCC_64_CMAKE_DIR}/gcc_64/lib/cmake/Qt5")
    message(STATUS "Qt5_DIR: ${Qt5_DIR}")
  endif()
elseif(APPLE)
  set(CODE_PREFIX_ "/Users/$ENV{USER}")
  find_path(Qt5_CLANG_64_CMAKE_DIR "clang_64/lib/cmake/Qt5/Qt5Config.cmake"
            PATHS "/Users/$ENV{USER}/Qt/*" "/Users/$ENV{USER}/code/src/Qt/*"
            NO_DEFAULT_PATH)
  if(EXISTS ${Qt5_CLANG_64_CMAKE_DIR})
    set(Qt5_DIR ${Qt5_CLANG_64_CMAKE_DIR})
    message(STATUS "Qt5_DIR: ${Qt5_DIR}")
  endif()
else()
  set(SYSTEM_DETECTED OFF)
endif()

if(SYSTEM_DETECTED)
  message(STATUS "CODE_PREFIX_=${CODE_PREFIX_}")

  ## mio
  set(MIO_INCLUDE_DIR "${CODE_PREFIX_}/code/src")
  add_definitions(-DMIO_INCLUDE_DIR="${MIO_INCLUDE_DIR}")

  ## OpenCV
  set(OpenCV_DIR "${CODE_PREFIX_}/code/install/opencv/dev/rel/share/OpenCV")

else()
  message(WARNING "Couldn't detect system type in PkgConfigPath.cmake")
endif()

