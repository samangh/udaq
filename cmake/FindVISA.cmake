if(WIN32)
  set(IVI_DIR "C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT")

  find_path(VISA_INCLUDE_DIRS
    NAMES visa.h
    HINTS ${VISA_DIR}/include ${IVI_DIR}/include)

  find_library(VISA_LIBRARIES
    NAMES visa64 visa32
    HINTS ${VISA_DIR}/lib ${VISA_DIR}/lib/msc ${VISA_DIR}/Lib_x64/msc ${IVI_DIR}/lib/msc ${IVI_DIR}/Lib_x64/msc)
endif()

if(APPLE)
  set(VISA_FRAMEWORK "/Library/Frameworks/VISA.framework")

  # The CMake searching for Frameworks is broken, so don't use. Instead
  # give direct paths to the library and header forlder.
  #
  # See https://cmake.org/pipermail/cmake/2014-April/057397.html
  set(CMAKE_FIND_FRAMEWORK NEVER)

  # set(VISA_LIBRARIES "${VISA_DIR}/VISA")

  find_path(VISA_INCLUDE_DIRS
    NAMES visa.h
    HINTS ${VISA_DIR}/include ${VISA_FRAMEWORK}/Headers)

  find_library(VISA_LIBRARIES
    NAMES VISA
    HINTS ${VISA_DIR}/lib ${VISA_FRAMEWORK})
endif()

if (UNIX AND NOT APPLE)
  find_path(VISA_INCLUDE_DIRS
    NAMES visa.h
    HINTS ${VISA_DIR}/include "/usr/include/ni-visa" "/usr/include/rsvisa")

  find_path(VISA_LIBRARIES
    NAMES visa64 visa32 rsvisa
    HINTS
      ${VISA_DIR}/lib

      # On openSUSE, CMake seems to search /usr/lib64 but not /usr/lib/x86_64-linux-gnu, so add those
      "/usr/lib/i386-linux-gnu" "/usr/lib/x86_64-linux-gnu")
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
