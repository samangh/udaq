set(TARGET "implot")
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${TARGET})

set(IMPLOT_DIR ${PROJECT_SOURCE_DIR}/external/implot)

##
## Source files
##

file(GLOB SRC_FILES
  ${IMPLOT_DIR}/*.cpp)
list(REMOVE_ITEM SRC_FILES ${IMPLOT_DIR}/implot_demo.cpp)

add_library(${TARGET} STATIC ${SRC_FILES})

##
## Includes
##

target_include_directories(${TARGET}
  INTERFACE
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${IMPLOT_DIR}>
  PRIVATE)

target_link_libraries(${TARGET}
  INTERFACE
  PUBLIC
    imgui
  PRIVATE)

##
## Compiler flags
##

target_compile_features(${TARGET} PRIVATE cxx_std_17)

