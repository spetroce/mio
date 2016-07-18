# - Try to find Xeneth
# Once done this will define
#  XENETH_FOUND - System has Xeneth
#  XENETH_INCLUDE_DIRS - The Xeneth include directories
#  XENETH_LIBRARIES - The libraries needed to use Xeneth
#  XENETH_DEFINITIONS - Compiler switches required for using Xeneth

find_package(PkgConfig)
pkg_check_modules(PC_XENETH QUIET libxeneth)
set(XENETH_DEFINITIONS ${PC_XENETH_CFLAGS_OTHER})

find_path(XENETH_INCLUDE_DIR XCamera.h
          HINTS ${PC_XENETH_INCLUDEDIR} ${PC_XENETH_INCLUDE_DIRS}
          PATHS /usr/share/xeneth/Include)

find_library(XENETH_LIBRARY xeneth
             HINTS ${PC_XENETH_LIBDIR} ${PC_XENETH_RARY_DIRS} 
             PATHS /usr/lib)

set(XENETH_LIBRARIES ${XENETH_LIBRARY} )
set(XENETH_INCLUDE_DIRS ${XENETH_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set XENETH_FOUND to TRUE if all listed variables are TRUE
find_package_handle_standard_args(Xeneth  DEFAULT_MSG XENETH_LIBRARY XENETH_INCLUDE_DIR)

mark_as_advanced(XENETH_INCLUDE_DIR XENETH_LIBRARY)

