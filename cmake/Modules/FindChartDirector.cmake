# - Try to find ChartDirector
# Once done this will define
#  CHART_DIRECTOR_FOUND - System has ChartDirector
#  CHART_DIRECTOR_INCLUDE_DIRS - The ChartDirector include directories
#  CHART_DIRECTOR_LIBRARIES - The libraries needed to use ChartDirector
#  CHART_DIRECTOR_DEFINITIONS - Compiler switches required for using ChartDirector

find_package(PkgConfig)
pkg_check_modules(PC_CHART_DIRECTOR QUIET libchartdir)
set(CHART_DIRECTOR_DEFINITIONS ${PC_CHART_DIRECTOR_CFLAGS_OTHER})

find_path(CHART_DIRECTOR_INCLUDE_DIR chartdir.h
          HINTS ${PC_CHART_DIRECTOR_INCLUDEDIR} ${PC_CHART_DIRECTOR_INCLUDE_DIRS}
          PATHS /usr/local/include/ChartDirector)

find_library(CHART_DIRECTOR_LIBRARY chartdir
             HINTS ${PC_CHART_DIRECTOR_LIBDIR} ${PC_CHART_DIRECTOR_LIBRARY_DIRS}
             PATHS /usr/local/lib64/ChartDirector)

set(CHART_DIRECTOR_LIBRARIES ${CHART_DIRECTOR_LIBRARY} )
set(CHART_DIRECTOR_INCLUDE_DIRS ${CHART_DIRECTOR_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set CHART_DIRECTOR_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ChartDirector  DEFAULT_MSG
                                  CHART_DIRECTOR_LIBRARY CHART_DIRECTOR_INCLUDE_DIR)

mark_as_advanced(CHART_DIRECTOR_INCLUDE_DIR CHART_DIRECTOR_LIBRARY )
