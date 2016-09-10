# - Try to find Qwt
# Once done this will define
#  QWT_FOUND - System has Qwt
#  QWT_INCLUDE_DIRS - The Qwt include directories
#  QWT_LIBRARIES - The libraries needed to use Qwt
#  QWT_DEFINITIONS - Compiler switches required for using Qwt

find_package(PkgConfig)
pkg_check_modules(PC_QWT QUIET Qt5Qwt6)
set(QWT_DEFINITIONS ${PC_QWT_CFLAGS_OTHER})

find_path(QWT_INCLUDE_DIR qwt_plot.h
          HINTS ${PC_QWT_INCLUDEDIR} ${PC_QWT_INCLUDE_DIRS}
          PATH_SUFFIXES qwt)

find_library(QWT_LIBRARY NAMES qwt libqwt qwt-qt5
             HINTS ${PC_QWT_LIBDIR} ${PC_QWT_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set QWT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Qwt DEFAULT_MSG
                                  QWT_LIBRARY QWT_INCLUDE_DIR)

mark_as_advanced(QWT_INCLUDE_DIR QWT_LIBRARY )

set(QWT_LIBRARIES ${QWT_LIBRARY} )
set(QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )
