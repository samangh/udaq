#include <stdio.h>
#include <iostream>
#include <vector>

#include <imgui-wrapper.h>
#include <imgui_internal.h>
#include <imfilebrowser.h>

#include <implot.h>
#include <cstdlib>
#include <version.h>
#include "karla-font.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <algorithm>

#include <udaq/devices/safibra/sigproc_server.h>
#include <udaq/common/vector.h>
#include <udaq/common/file_writer.h>
#include <udaq/helpers/imgui.h>

#include <cmath>
#include <fmt/format.h>

class MyContext {
  public:
    SDL_Window *window;
    SDL_GLContext gl_context;
    ImGuiIO io;
};

void clean_up(MyContext &context) {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(context.gl_context);
    SDL_DestroyWindow(context.window);
    SDL_Quit();
}

MyContext initialise() {
    MyContext imgui_context;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        std::cout << "Error: " << SDL_GetError() << std::endl;
        exit(-1);
    }

#ifdef __APPLE__
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    imgui_context.window = SDL_CreateWindow("uDAQ",  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    imgui_context.gl_context = SDL_GL_CreateContext(imgui_context.window);
    SDL_GL_MakeCurrent(imgui_context.window, imgui_context.gl_context);

    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    if (gl3wInit() != 0)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        exit(-1);
    }

     // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
   imgui_context.io = ImGui::GetIO();
      (void)imgui_context.io;

    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
        Karla_compressed_data, karla_compressed_size, 14);
    imgui_context.io.Fonts->Build();

    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 5.0f;


    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(imgui_context.window, imgui_context.gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return imgui_context;
}

void disable_item(bool visible, std::function<void(void)> func)
{
    if (visible)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        func();
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    else
        func();
}

#ifdef WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int, char**)
{

    auto imgui_context=initialise();
    //ImPlot::GetStyle().AntiAliasedLines = true;

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    // Main loop
    bool done = false;

    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(imgui_context.window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(imgui_context.window);
        ImGui::NewFrame();

        

        //Menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                done = ImGui::MenuItem("Exit");
                ImGui::EndMenu();
            }

            ImGui::SameLine(ImGui::GetIO().DisplaySize.x - 60.f);
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            ImGui::EndMainMenuBar();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)imgui_context.io.DisplaySize.x, (int)imgui_context.io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(imgui_context.window);
    }

    clean_up(imgui_context);

    return 0;
}
