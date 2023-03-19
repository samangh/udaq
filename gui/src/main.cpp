#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <version.h>
#include <atomic>


#include <sg/imgui/imgui_wrapper_sdl2_opengl3.h>
#include <fmt/format.h>

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

// Main code
int main(int, char**)
{

    sg::imgui::IImGuiWrapper::on_start_t start = [](){};
    sg::imgui::IImGuiWrapper::on_end_t end = [](){};
    sg::imgui::IImGuiWrapper::on_iteration_t ter = [](bool&){
        ImGui::ShowDemoWindow();
    };
    sg::imgui::ImGuiWrapper_Sdl2_OpenGl3 wrap = sg::imgui::ImGuiWrapper_Sdl2_OpenGl3(start, end, ter);
    wrap.start("Hello");

    return 0;
}
