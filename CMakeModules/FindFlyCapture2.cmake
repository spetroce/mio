# - Try to find FlyCapture2
# Once done this will define
#  FLY_CAPTURE_2_FOUND - System has FlyCapture2
#  FLY_CAPTURE_2_INCLUDE_DIRS - The FlyCapture2 include directories
#  FLY_CAPTURE_2_LIBRARIES - The libraries needed to use FlyCapture2
#  FLY_CAPTURE_2_DEFINITIONS - Compiler switches required for using FlyCapture2

find_package(PkgConfig)
pkg_check_modules(PC_FLY_CAPTURE_2 QUIET FlyCapture2)
set(FLY_CAPTURE_2_DEFINITIONS ${PC_FLY_CAPTURE_2_CFLAGS_OTHER})

find_path(FLY_CAPTURE_2_INCLUDE_DIR FlyCapture2.h
          HINTS ${PC_FLY_CAPTURE_2_INCLUDEDIR} ${PC_FLY_CAPTURE_2_INCLUDE_DIRS}
          PATH_SUFFIXES flycapture)

find_library(FLY_CAPTURE_2_LIBRARY NAMES flycapture libflycapture
             HINTS ${PC_FLY_CAPTURE_2_LIBDIR} ${PC_FLY_CAPTURE_2_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FLY_CAPTURE_2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(FlyCapture2 DEFAULT_MSG
                                  FLY_CAPTURE_2_LIBRARY FLY_CAPTURE_2_INCLUDE_DIR)

mark_as_advanced(FLY_CAPTURE_2_INCLUDE_DIR FLY_CAPTURE_2_LIBRARY )

set(FLY_CAPTURE_2_LIBRARIES ${FLY_CAPTURE_2_LIBRARY} )
set(FLY_CAPTURE_2_INCLUDE_DIRS ${FLY_CAPTURE_2_INCLUDE_DIR} )
