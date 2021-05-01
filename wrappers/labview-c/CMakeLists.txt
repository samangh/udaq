set(TARGET "labview-c-wrapper")
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${TARGET})

find_package(LabVIEW REQUIRED)

##
## Source files
##

file(GLOB_RECURSE SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# This MUST be a shared library, as otherwise you can't export from it
# See https://devblogs.microsoft.com/oldnewthing/20140321-00/?p=1433
add_library(${TARGET} SHARED ${SRC_FILES})

##
## Includes
##

target_include_directories(${TARGET}
  INTERFACE
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
  PRIVATE)

target_link_libraries(${TARGET}
  INTERFACE
  PUBLIC
   LabVIEW
   $<$<CXX_COMPILER_ID:MSVC>:-SAFESEH:NO>
  PRIVATE
    safibra
    common
    "$<$<BOOL:${APPLE}>:-framework CoreFoundation>"
	)

# The LabVIEW Windows libraries have SAFESEH disabled, so to link we have to do the same
if(MSVC)
	target_link_options(${TARGET} PUBLIC "/SAFESEH:NO")
endif()

include (GenerateExportHeader)
generate_export_header(${TARGET}
  EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/include/${PROJECT_NAME}/export/${TARGET}.h")

set_target_properties(${TARGET} PROPERTIES
  FRAMEWORK TRUE
  VERSION ${PROJECT_VERSION}
  SOVERSION  ${PROJECT_VERSION})

setup_ide_folders(TARGET ${TARGET} DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_compile_definitions(${TARGET} PUBLIC X64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  target_compile_definitions(${TARGET} PUBLIC X86)
endif()


##
## Compiler flags
##

target_compile_features(${TARGET} PRIVATE cxx_std_17)

##
## Version
##

configure_file (
  "${PROJECT_SOURCE_DIR}/cmake/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  )

