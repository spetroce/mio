# - Try to find FlyCapture2
# Once done this will define
#  FLYCAPTURE2_FOUND - System has FlyCapture2
#  FLYCAPTURE2_INCLUDE_DIRS - The FlyCapture2 include directories
#  FLYCAPTURE2_LIBRARIES - The libraries needed to use FlyCapture2
#  FLYCAPTURE2_DEFINITIONS - Compiler switches required for using FlyCapture2

find_package(PkgConfig)
pkg_check_modules(PC_FLYCAPTURE2 QUIET FlyCapture2)
set(FLYCAPTURE2_DEFINITIONS ${PC_FLYCAPTURE2_CFLAGS_OTHER})

find_path(FLYCAPTURE2_INCLUDE_DIR FlyCapture2.h
          HINTS ${PC_FLYCAPTURE2_INCLUDEDIR} ${PC_FLYCAPTURE2_INCLUDE_DIRS}
          PATH_SUFFIXES flycapture)

find_library(FLYCAPTURE2_LIBRARY NAMES flycapture libflycapture
             HINTS ${PC_FLYCAPTURE2_LIBDIR} ${PC_FLYCAPTURE2_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FLYCAPTURE2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(FlyCapture2 DEFAULT_MSG
                                  FLYCAPTURE2_LIBRARY FLYCAPTURE2_INCLUDE_DIR)

mark_as_advanced(FLYCAPTURE2_INCLUDE_DIR FLYCAPTURE2_LIBRARY )

set(FLYCAPTURE2_LIBRARIES ${FLYCAPTURE2_LIBRARY} )
set(FLYCAPTURE2_INCLUDE_DIRS ${FLYCAPTURE2_INCLUDE_DIR} )
