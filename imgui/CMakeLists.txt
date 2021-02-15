set(TARGET "imgui")
set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${TARGET})

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/external/imgui)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

##
## Source files
##

file(GLOB SRC_FILES
  ${IMGUI_DIR}/*.cpp
  ${IMGUI_DIR}/examples/libs/gl3w/GL/*.c
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp)

list(REMOVE_ITEM SRC_FILES ${IMGUI_DIR}/imgui_demo.cpp)

add_library(${TARGET} STATIC ${SRC_FILES})

##
## Includes
##

target_include_directories(${TARGET}
  INTERFACE
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${IMGUI_DIR}>
    $<BUILD_INTERFACE:${IMGUI_DIR}/examples/libs/gl3w>
  PRIVATE)

# Old versions of SDL2 don't export namekspace targets
if(TARGET SDL2::SDL2)
  target_link_libraries(${TARGET} PUBLIC
    $<IF:$<BOOL:${BUILD_SHARED_LIBS}>,SDL2::SDL2,SDL2::SDL2-static>
    SDL2::SDL2main)
else()
  target_include_directories(${TARGET} PUBLIC ${SDL2_INCLUDE_DIRS})
  target_link_libraries(${TARGET} PUBLIC ${SDL2_LIBRARIES})
endif()

target_link_libraries(${TARGET}
  INTERFACE
  PUBLIC
  PRIVATE
    OpenGL::GL
    "$<$<BOOL:${APPLE}>:-framework CoreFoundation>"
    $<$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>:${CMAKE_DL_LIBS}>)

##
## Compiler flags
##

target_compile_features(${TARGET} PRIVATE cxx_std_17)

##
## Version
##

configure_file (
  "${PROJECT_SOURCE_DIR}/cmake/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h")

