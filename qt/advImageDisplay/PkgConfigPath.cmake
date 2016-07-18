set(SYSTEM_DETECTED ON)
if(UNIX AND NOT APPLE)
  set(CODE_PREFIX_ "/home/$ENV{USER}")
elseif(APPLE)
  set(CODE_PREFIX_ "/Users/$ENV{USER}")
  set(Qt5_DIR "/Users/$ENV{USER}/Qt/5.7/clang_64/lib/cmake/Qt5")
else()
  set(SYSTEM_DETECTED OFF)
endif()

if(SYSTEM_DETECTED)
  message(STATUS "CODE_PREFIX_=${CODE_PREFIX_}")

  ## mio
  set(MIO_INCLUDE_DIR "${CODE_PREFIX_}/code/src")

  ## OpenCV
  set(OpenCV_DIR "${CODE_PREFIX_}/code/install/opencv/dev/rel/share/OpenCV")

else()
  message(WARNING "Couldn't detect system type in PkgConfigPath.cmake")
endif()

