# - Try to find levmar
# Once done this will define
#  LEVMAR_FOUND - System has levmar
#  LEVMAR_INCLUDE_DIRS - The levmar include directories
#  LEVMAR_LIBRARIES - The libraries needed to use levmar
#  LEVMAR_DEFINITIONS - Compiler switches required for using levmar

find_package(PkgConfig)
pkg_check_modules(PC_LEVMAR QUIET levmar)
set(LEVMAR_DEFINITIONS ${PC_LEVMAR_CFLAGS_OTHER})

find_path(LEVMAR_INCLUDE_DIR levmar.h
          HINTS ${PC_LEVMAR_INCLUDEDIR} ${PC_LEVMAR_INCLUDE_DIRS})

find_library(LEVMAR_LIBRARY NAMES levmar liblevmar
             HINTS ${PC_LEVMAR_LIBDIR} ${PC_LEVMAR_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LEVMAR_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(levmar DEFAULT_MSG
                                  LEVMAR_LIBRARY LEVMAR_INCLUDE_DIR)

mark_as_advanced(LEVMAR_INCLUDE_DIR LEVMAR_LIBRARY)

set(LEVMAR_LIBRARIES ${LEVMAR_LIBRARY})
set(LEVMAR_INCLUDE_DIRS ${LEVMAR_INCLUDE_DIR})
