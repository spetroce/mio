# - Try to find eBUS
# Once done this will define
#  EBUS_FOUND - System has eBUS
#  EBUS_INCLUDE_DIRS - The eBUS include directories
#  EBUS_LIBRARIES - The libraries needed to use eBUS
#  EBUS_DEFINITIONS - Compiler switches required for using eBUS

set(EBUS_LIBS_  apr-1
                aprutil-1
                EbTransportLayerLib
                EbUtilsLib
                log4cxx
                PtConvertersLib
                PtUtilsLib
                PvAppUtils
                PvBase
                PvBuffer
                PvDevice
                PvGenICam
                PvGUI
                PvPersistence
                PvSerial
                PvStream
                PvTransmitter
                PvVirtualDevice
                SimpleImagingLib)

find_package(PkgConfig)
pkg_check_modules(PC_EBUS QUIET ebus_sdk)
set(EBUS_DEFINITIONS ${PC_EBUS_CFLAGS_OTHER})

find_path(EBUS_INCLUDE_DIR NAMES PvBase.h
          HINTS ${PC_EBUS_INCLUDEDIR} ${PC_EBUS_INCLUDE_DIRS}
          PATHS /opt/pleora/ebus_sdk/*/include)

find_library(EBUS_LIBRARY_ NAMES PvBase
             HINTS ${PC_EBUS_LIBDIR} ${PC_EBUS_LIBRARY_DIRS}
             PATHS /opt/pleora/ebus_sdk/*/lib)

get_filename_component(EBUS_LIB_DIR ${EBUS_LIBRARY_} DIRECTORY)
set(EBUS_LIBRARIES "")
foreach(EBUS_LIB_ ${EBUS_LIBS_})
  set(EBUS_LIBRARIES ${EBUS_LIBRARIES} ${EBUS_LIB_DIR}/lib${EBUS_LIB_}.so)
endforeach()

#set(EBUS_LIBRARIES ${EBUS_LIBRARY} )
set(EBUS_INCLUDE_DIRS ${EBUS_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set EBUS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(eBUS DEFAULT_MSG
                                  EBUS_LIBRARY EBUS_INCLUDE_DIR)

mark_as_advanced(EBUS_INCLUDE_DIR EBUS_LIBRA)

