# Find Dislin
#
# This sets the following variables:
# DISLIN_FOUND - True if DISLIN was found.
# DISLIN_INCLUDE_DIRS - Directories containing the DISLIN include files.
# DISLIN_LIBRARIES - Libraries needed to use DISLIN.
# DISLIN_DEFINITIONS - Compiler flags for DISLIN.

find_package(PkgConfig)
pkg_check_modules(PC_DISLIN dislin)
set(DISLIN_DEFINITIONS ${PC_DISLIN_CFLAGS_OTHER})

#libxm.so.4 was required.  Ubuntu no longer supplies this library directly.  sudo apt-get install libmotif to get it

#header found in /usr/local/dislin
find_path(DISLIN_INCLUDE_DIR dislin.h
    HINTS ${PC_DISLIN_INCLUDEDIR}/dislin ${PC_DISLIN_INCLUDE_DIRS}/dislin
    PATHS /usr/include /usr/local/include /usr/local/dislin)

#library found in both /usr/lib/libdislin.so.10 /usr/local/dislin/libdislin.so.10
find_library(DISLIN_LIBRARY dislin
    HINTS ${PC_DISLIN_LIBDIR} ${PC_DISLIN_LIBRARY_DIRS} 
    PATHS /usr/lib /usr/local/lib64 /usr/local/dislin)

set(DISLIN_INCLUDE_DIRS ${DISLIN_INCLUDE_DIR})
set(DISLIN_LIBRARIES ${DISLIN_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Dislin DEFAULT_MSG DISLIN_LIBRARY DISLIN_INCLUDE_DIR)

mark_as_advanced(DISLIN_LIBRARY DISLIN_INCLUDE_DIR)

