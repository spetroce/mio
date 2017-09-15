set(SYSTEM_DETECTED ON)
if(UNIX AND NOT APPLE)
  set(CODE_PREFIX_ "/home/$ENV{USER}")
  set(Qt5_DIR_LINUX "/home/$ENV{USER}/Qt/5.9.1/gcc_64/lib/cmake/Qt5")
  if(EXISTS ${Qt5_DIR_LINUX})
    set(Qt5_DIR ${Qt5_DIR_LINUX})
    message(STATUS "Qt5_DIR: ${Qt5_DIR}")
  endif()
elseif(APPLE)
  set(CODE_PREFIX_ "/Users/$ENV{USER}")
  set(Qt5_DIR_APPLE "/Users/$ENV{USER}/Qt/5.9.1/clang_64/lib/cmake/Qt5")
  if(EXISTS ${Qt5_DIR_APPLE})
    set(Qt5_DIR ${Qt5_DIR_APPLE})
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

