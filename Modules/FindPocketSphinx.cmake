# - Try to find pocketsphinx
# Once done this will define
#  POCKETSPHINX_FOUND - System has pocketsphinx
#  POCKETSPHINX_INCLUDE_DIRS - The pocketsphinx include directories
#  POCKETSPHINX_LIBRARIES - The libraries needed to use pocketsphinx
#  POCKETSPHINX_DEFINITIONS - Compiler switches required for using pocketsphinx

find_package(PkgConfig)
pkg_check_modules(PC_POCKETSPHINX QUIET pocketsphinx)
set(POCKETSPHINX_DEFINITIONS ${PC_POCKETSPHINX_CFLAGS_OTHER})

find_path(POCKETSPHINX_INCLUDE_DIR pocketsphinx/pocketsphinx.h
          HINTS ${PC_POCKETSPHINX_INCLUDEDIR} ${PC_POCKETSPHINX_INCLUDE_DIRS})

find_library(POCKETSPHINX_LIBRARY NAMES pocketsphinx libpocketsphinx
             HINTS ${PC_POCKETSPHINX_LIBDIR} ${PC_POCKETSPHINX_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set POCKETSPHINX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(pocketsphinx DEFAULT_MSG
                                  POCKETSPHINX_LIBRARY POCKETSPHINX_INCLUDE_DIR)

mark_as_advanced(POCKETSPHINX_INCLUDE_DIR POCKETSPHINX_LIBRARY)

set(POCKETSPHINX_LIBRARIES ${POCKETSPHINX_LIBRARY})
set(POCKETSPHINX_INCLUDE_DIRS ${POCKETSPHINX_INCLUDE_DIR} "${POCKETSPHINX_INCLUDE_DIR}/pocketsphinx")

