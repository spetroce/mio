# - Try to find LibQGLViewer
# Once done this will define
#  LIBQGLVIEWER_FOUND - System has LibQGLViewer
#  LIBQGLVIEWER_INCLUDE_DIRS - The LibQGLViewer include directories
#  LIBQGLVIEWER_LIBRARIES - The libraries needed to use LibQGLViewer
#  LIBQGLVIEWER_DEFINITIONS - Compiler switches required for using LibQGLViewer

find_package(PkgConfig)
pkg_check_modules(PC_LIBQGLVIEWER QUIET libQGLViewer)
set(LIBQGLVIEWER_DEFINITIONS ${PC_LIBQGLVIEWER_CFLAGS_OTHER})

find_path(LIBQGLVIEWER_INCLUDE_DIR QGLViewer/qglviewer.h
          HINTS ${PC_LIBQGLVIEWER_INCLUDEDIR} ${PC_LIBQGLVIEWER_INCLUDE_DIRS}
          PATH_SUFFIXES libQGLViewer)

find_library(LIBQGLVIEWER_LIBRARY NAMES QGLViewer libQGLViewer
             HINTS ${PC_LIBQGLVIEWER_LIBDIR} ${PC_LIBQGLVIEWER_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBQGLVIEWER_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibQGLViewer DEFAULT_MSG
                                  LIBQGLVIEWER_LIBRARY LIBQGLVIEWER_INCLUDE_DIR)

mark_as_advanced(LIBQGLVIEWER_INCLUDE_DIR LIBQGLVIEWER_LIBRARY )

set(LIBQGLVIEWER_LIBRARIES ${LIBQGLVIEWER_LIBRARY} )
set(LIBQGLVIEWER_INCLUDE_DIRS ${LIBQGLVIEWER_INCLUDE_DIR} )

