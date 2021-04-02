set(TARGET "imgui-filebrowser")

##
## Source files
##

add_library(${TARGET} INTERFACE)

##
## Includes
##

target_include_directories(${TARGET}
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/imgui-filebrowser>
    imgui)

