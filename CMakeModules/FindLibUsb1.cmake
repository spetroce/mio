# - Try to find LibUsb1
# Once done this will define
#  LIBUSB1_FOUND - System has LibUsb1
#  LIBUSB1_INCLUDE_DIRS - The LibUsb1 include directories
#  LIBUSB1_LIBRARIES - The libraries needed to use LibUsb1
#  LIBUSB1_DEFINITIONS - Compiler switches required for using LibUsb1

find_package(PkgConfig)
pkg_check_modules(PC_LIBUSB1 QUIET libusb-1.0)
set(LIBUSB1_DEFINITIONS ${PC_LIBUSB1_CFLAGS_OTHER})

find_path(LIBUSB1_INCLUDE_DIR NAMES libusb.h
          HINTS ${PC_LIBUSB1_INCLUDEDIR} ${PC_LIBUSB1_INCLUDE_DIRS}
          PATHS /usr/include/libusb-1.0 /usr/local/include/libusb-1.0)

find_library(LIBUSB1_LIBRARY NAMES usb-1.0
             HINTS ${PC_LIBUSB1_LIBDIR} ${PC_LIBUSB1_LIBRARY_DIRS}
             PATHS /usr/local/lib64 /usr/local/lib)

set(LIBUSB1_LIBRARIES ${LIBUSB1_LIBRARY} )
set(LIBUSB1_INCLUDE_DIRS ${LIBUSB1_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBUSB1_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibUsb1 DEFAULT_MSG
                                  LIBUSB1_LIBRARY LIBUSB1_INCLUDE_DIR)

mark_as_advanced(LIBUSB1_INCLUDE_DIR LIBUSB1_LIBRA)
	
