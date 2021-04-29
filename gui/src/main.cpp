#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <version.h>
#include <atomic>

#include <imgui-wrapper.h>
#include <imgui_internal.h>
#include <imfilebrowser.h>
#include <implot.h>
#include <fmt/format.h>

#include <udaq/helpers/imgui.h>

#include "gui_context.h"

#ifdef WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int, char**)
{

    MyContext imgui_context;
    imgui_context.initialise("uDAQ");

    // Our state
    imgui_context.start_gui_loop([&imgui_context](){
        ImGui::NewFrame();
        
        //Menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit"))
                    imgui_context.stop_gui_loop();
                ImGui::EndMenu();
            }

            ImGui::SameLine(ImGui::GetIO().DisplaySize.x - 60.f);
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            ImGui::EndMainMenuBar();
        }

    });

    imgui_context.cleanup();

    return 0;
}
