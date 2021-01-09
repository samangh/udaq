if(WIN32 AND NOT VISA_DIR)
  set(VISA_DIR "C:\\Program Files (x86)\\IVI Foundation\\VISA\\WinNT")
  set(WIN32_LIBRARY_SEARCH_HINTS ${VISA_DIR}/lib/msc ${VISA_DIR}/Lib_x64/msc)
endif()

FIND_PATH(VISA_INCLUDE_DIRS
  NAMES visa.h
  HINTS ${VISA_DIR}/include /usr/include/ni-visa)

# On openSUSE, CMake seems to seach /usr/lib64 but not /usr/lib/x86_64-linux-gnu, so add those
FIND_LIBRARY(VISA_LIBRARIES
  NAMES visa visa64 visa32
  HINTS ${VISA_DIR}/lib /usr/lib/i386-linux-gnu /usr/lib/x86_64-linux-gnu ${WIN32_LIBRARY_SEARCH_HINTS})

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VISA DEFAULT_MSG VISA_LIBRARIES VISA_INCLUDE_DIRS)

if(VISA_FOUND AND NOT TARGET VISA)
  add_library(VISA UNKNOWN IMPORTED)
  set_target_properties(VISA PROPERTIES
	IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
	IMPORTED_LOCATION "${VISA_LIBRARIES}"
	INTERFACE_INCLUDE_DIRECTORIES "${VISA_INCLUDE_DIRS}"
	)
endif()
