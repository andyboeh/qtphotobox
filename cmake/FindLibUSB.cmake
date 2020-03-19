# - Try to find libusb-1.0
# Once done, this will define
#
#  LIBUSB_FOUND - system has libusb-1.0
#  LIBUSB_INCLUDE_DIRS - the libusb-1.0 include directories
#  LIBUSB_LIBRARIES - link these to use libusb-1.0

include(LibFindMacros)

# Use pkg-config to get hints about paths
# libfind_pkg_check_modules(LIBUSB_PKGCONF libusb-1.0>=1.0.19)
libfind_pkg_check_modules(LIBUSB_PKGCONF libusb-1.0)

# Include dir
find_path(LIBUSB_INCLUDE_DIR
  NAMES libusb.h
  PATHS ${LIBUSB_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(LIBUSB_LIBRARY
  NAMES usb-1.0
  PATHS ${LIBUSB_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LIBUSB_PROCESS_INCLUDES LIBUSB_INCLUDE_DIR)
set(LIBUSB_PROCESS_LIBS LIBUSB_LIBRARY)
libfind_process(LIBUSB)
