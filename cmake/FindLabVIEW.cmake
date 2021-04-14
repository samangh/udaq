find_path(LabVIEW_INCLUDE_DIRS
   NAMES extcode.h
   HINTS ${LabVIEW_DIR} ${LabVIEW_DIR}/cintools ${LabVIEW_DIR}/include
	     "C:\\Program Files (x86)\\National Instruments\\LabVIEW 2020\\cintools"
		 "C:\\Program Files (x86)\\National Instruments\\LabVIEW 2018\\cintools")

find_library(LabVIEW_LIBRARIES
  NAMES labview
  HINTS ${LabVIEW_DIR} ${LabVIEW_DIR}/cintools ${LabVIEW_DIR}/lib
        "C:\\Program Files (x86)\\National Instruments\\LabVIEW 2020\\cintools"
		"C:\\Program Files (x86)\\National Instruments\\LabVIEW 2018\\cintools")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LabVIEW DEFAULT_MSG LabVIEW_LIBRARIES LabVIEW_INCLUDE_DIRS)

if(LabVIEW_FOUND AND NOT TARGET LabVIEW)
  add_library(LabVIEW UNKNOWN IMPORTED)
  set_target_properties(LabVIEW PROPERTIES
	IMPORTED_LINK_INTERFACE_LANGUAGES "C;CXX"
	IMPORTED_LOCATION "${LabVIEW_LIBRARIES}"
	INTERFACE_INCLUDE_DIRECTORIES "${LabVIEW_INCLUDE_DIRS}")
endif()