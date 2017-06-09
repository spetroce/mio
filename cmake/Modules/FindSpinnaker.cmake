# - Try to find Spinnaker
# Once done this will define
#  SPINNAKER_FOUND - System has Spinnaker
#  SPINNAKER_INCLUDE_DIRS - The Spinnaker include directories
#  SPINNAKER_LIBRARIES - The libraries needed to use Spinnaker
#  SPINNAKER_DEFINITIONS - Compiler switches required for using Spinnaker

#download spinnaker from ptgrey website
#extract and copy to /opt removing the version number
#eg., cp ~/Downloads/spinnaker_1_0_0_295_amd64 /opt/spinnaker

find_package(PkgConfig)
pkg_check_modules(PC_SPINNAKER QUIET Spinnaker)
set(SPINNAKER_DEFINITIONS ${PC_SPINNAKER_CFLAGS_OTHER})

find_path(SPINNAKER_INCLUDE_DIR Spinnaker.h
          HINTS ${PC_SPINNAKER_INCLUDEDIR} ${PC_SPINNAKER_INCLUDE_DIRS} 
          PATHS /opt/ /opt/spinnaker/include
          PATH_SUFFIXES spinnaker/include)

find_library(SPINNAKER_LIBRARY NAMES Spinnaker libSpinnaker
             HINTS ${PC_SPINNAKER_LIBDIR} ${PC_SPINNAKER_LIBRARY_DIRS}
             PATHS /opt/ /opt/spinnaker/lib
             PATH_SUFFIXES spinnaker/lib)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SPINNAKER_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Spinnaker DEFAULT_MSG
                                  SPINNAKER_LIBRARY SPINNAKER_INCLUDE_DIR)

mark_as_advanced(SPINNAKER_INCLUDE_DIR SPINNAKER_LIBRARY )

set(SPINNAKER_LIBRARIES ${SPINNAKER_LIBRARY})
set(SPINNAKER_INCLUDE_DIRS ${SPINNAKER_INCLUDE_DIR})

message(STATUS "SPINNAKER_LIBRARIES: ${SPINNAKER_LIBRARIES}")
message(STATUS "SPINNAKER_INCLUDE_DIRS: ${SPINNAKER_INCLUDE_DIRS}")

