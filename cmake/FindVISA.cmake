if(WIN32 AND NOT VISA_DIR)
    set(VISA_DIR "C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT")
endif()

if(APPLE)
  if (NOT VISA_DIR)
    set(VISA_DIR "/Library/Frameworks/VISA.framework")
  endif()

  # The CMake searching for Frameworks is broken, so don't use. Instead
  # give direct paths to the library and header forlder.
  #
  # See https://cmake.org/pipermail/cmake/2014-April/057397.html
  set(CMAKE_FIND_FRAMEWORK NEVER)

  set(VISA_LIBRARIES "${VISA_DIR}/VISA")
  set(VISA_INCLUDE_DIRS "${VISA_DIR}/Headers")
endif()

if (NOT VISA_INCLUDE_DIRS)
  FIND_PATH(VISA_INCLUDE_DIRS
    NAMES visa.h
    HINTS ${VISA_DIR}/include "/usr/include/ni-visa" "/usr/include/rsvisa")
endif()

if (NOT VISA_LIBRARIES)
  FIND_LIBRARY(VISA_LIBRARIES
    NAMES visa64 visa32 rsvisa
    HINTS
      ${VISA_DIR}/lib

      # On openSUSE, CMake seems to search /usr/lib64 but not /usr/lib/x86_64-linux-gnu, so add those
      "/usr/lib/i386-linux-gnu" "/usr/lib/x86_64-linux-gnu"

      # Windows hints
      ${VISA_DIR}/lib/msc ${VISA_DIR}/Lib_x64/msc)
endif()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VISA DEFAULT_MSG VISA_LIBRARIES VISA_INCLUDE_DIRS)

if(VISA_FOUND AND NOT TARGET VISA)
  add_library(VISA UNKNOWN IMPORTED)
  set_target_properties(VISA PROPERTIES
	IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
	IMPORTED_LOCATION "${VISA_LIBRARIES}"
	INTERFACE_INCLUDE_DIRECTORIES "${VISA_INCLUDE_DIRS}")
endif()
