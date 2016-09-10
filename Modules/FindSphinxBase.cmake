# - Try to find sphinxbase
# Once done this will define
#  SPHINXBASE_FOUND - System has sphinxbase
#  SPHINXBASE_INCLUDE_DIRS - The sphinxbase include directories
#  SPHINXBASE_LIBRARIES - The libraries needed to use sphinxbase
#  SPHINXBASE_DEFINITIONS - Compiler switches required for using sphinxbase

find_package(PkgConfig)
pkg_check_modules(PC_SPHINXBASE QUIET sphinxbase)
set(SPHINXBASE_DEFINITIONS ${PC_SPHINXBASE_CFLAGS_OTHER})

find_path(SPHINXBASE_INCLUDE_DIR sphinxbase/err.h
          HINTS ${PC_SPHINXBASE_INCLUDEDIR} ${PC_SPHINXBASE_INCLUDE_DIRS})

find_library(SPHINXBASE_LIBRARY NAMES sphinxbase libsphinxbase
             HINTS ${PC_SPHINXBASE_LIBDIR} ${PC_SPHINXBASE_LIBRARY_DIRS})

find_library(SPHINXAD_LIBRARY NAMES sphinxad libsphinxad
             HINTS ${PC_SPHINXBASE_LIBDIR} ${PC_SPHINXBASE_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SPHINXBASE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(sphinxbase DEFAULT_MSG
                                  SPHINXBASE_LIBRARY SPHINXAD_LIBRARY SPHINXBASE_INCLUDE_DIR)

mark_as_advanced(SPHINXBASE_INCLUDE_DIR SPHINXBASE_LIBRARY SPHINXAD_LIBRARY)

set(SPHINXBASE_LIBRARIES ${SPHINXBASE_LIBRARY} ${SPHINXAD_LIBRARY})
set(SPHINXBASE_INCLUDE_DIRS ${SPHINXBASE_INCLUDE_DIR} "${SPHINXBASE_INCLUDE_DIR}/sphinxbase")
