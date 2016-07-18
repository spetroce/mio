# - Try to find lcm
# Once done this will define
#  LCM_FOUND - System has lcm
#  LCM_INCLUDE_DIRS - The lcm include directories
#  LCM_LIBRARIES - The libraries needed to use lcm
#  LCM_DEFINITIONS - Compiler switches required for using lcm

find_package(PkgConfig)
pkg_check_modules(PC_LCM QUIET lcm)
set(LCM_DEFINITIONS ${PC_LCM_CFLAGS_OTHER})

find_path(LCM_INCLUDE_DIR lcm/lcm.h
          HINTS ${PC_LCM_INCLUDEDIR} ${PC_LCM_INCLUDE_DIRS}
          PATH_SUFFIXES lcm)

find_library(LCM_LIBRARY NAMES lcm liblcm
             HINTS ${PC_LCM_LIBDIR} ${PC_LCM_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LCM_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(lcm DEFAULT_MSG
                                  LCM_LIBRARY LCM_INCLUDE_DIR)

mark_as_advanced(LCM_INCLUDE_DIR LCM_LIBRARY)

set(LCM_LIBRARIES ${LCM_LIBRARY})
set(LCM_INCLUDE_DIRS ${LCM_INCLUDE_DIR})

