#include <stdio.h>
#include <iostream>
#include <vector>

#include <imgui-wrapper.h>
#include <imgui_internal.h>

#include <implot.h>
#include <cstdlib>
#include <version.h>
#include "karla-font.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>

#include <udaq/devices/safibra/sigproc_server.h>

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

void do_work(std::vector<double>& _x, std::vector<double>& _y, bool& abort, std::shared_mutex& _mutex) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        std::unique_lock lock(_mutex);
        _x.push_back(_x.size());
        _y.push_back(_y.size());
        if (abort)
            break;
    };
}

void disable_item(bool visible, std::function<void(void)> func)
{
    if (!visible)
    {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    func();
    if (!visible)
    {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
}

int main(int, char**)
{
    auto imgui_context=initialise();
    //ImPlot::GetStyle().AntiAliasedLines = true;

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::vector<double> xs;
    std::vector<double> y;
    bool abort = false;
    std::shared_mutex mutex_;


    // Main loop
    bool done = false;
    bool updatePlot=true;
    bool autoScale = true;

    std::atomic<bool> error_received = false;
    std::atomic<bool> client_connected = false;    
    std::atomic<bool> listening = false;   
    std::string error_message;

    auto on_client_connected = [&]() { client_connected = true; };
    auto on_client_disconnected = [&]() { client_connected = false; };
    auto on_server_started = [&]() { listening = true; };
    auto on_server_stopped = [&]() {
        listening = false;
        client_connected = false;
    };

    auto on_error = [&](const std::string message) {
        error_received = true;
        error_message = message;
    };

    std::thread t(do_work, std::ref(xs), std::ref(y), std::ref(abort), std::ref(mutex_));
    auto client = udaq::devices::safibra::SigprogServer(on_error, on_client_connected, on_client_disconnected, on_server_started, on_server_stopped);

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
            ImGui::EndMainMenuBar();
        }
                   
        ImGui::Begin("Interrogator Client",NULL, ImGuiWindowFlags_AlwaysAutoResize);
        {
            disable_item(!client.is_running(), [&](){
                if (ImGui::Button("Connect"))
                {
                    client.start(5555);
                }
            });
            ImGui::SameLine();
            disable_item(client.is_running(), [&]() {
                if (ImGui::Button("Disconnect"))
                    client.stop();
            });

            ImGui::RadioButton("Listening", listening);
            ImGui::RadioButton("Interrogator connected", client_connected);
        }
        ImGui::End();

        ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(10240, 10240));
        ImGui::Begin("Plot");
        {
            ImGui::Checkbox("Update plot", &updatePlot);
            ImGui::SameLine();
            ImGui::Checkbox("Autoscale", &autoScale);            
            ImGui::Text("Number of elements: %lu", xs.size());

            ImPlot::SetNextPlotLimits(0, xs.back() + 10, 0, y.back() + 10,
                                      autoScale ? ImGuiCond_::ImGuiCond_Always
                                                : ImGuiCond_::ImGuiCond_Once);
            if (ImPlot::BeginPlot("Log Plot", NULL, NULL, ImVec2(-1, -1),
                                  ImGuiCond_::ImGuiCond_Always)) {
                std::shared_lock lock(mutex_);
                ImPlot::PlotLine("f(x) = x", xs.data(), y.data(),
                                 (int)xs.size());
                ImPlot::EndPlot();
            }
        }
        ImGui::End();

        if (error_received)
        {
            ImGui::OpenPopup("Error");

           // Always center this window when appearing
           //ImVec2 center = ImGui::GetMainViewport()->GetCenter();
           //ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Error: %s", error_message.c_str());
                if (ImGui::Button("OK")) {
                    client.stop();
                    error_received = false;
                    
                }
                ImGui::EndPopup();
            }
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)imgui_context.io.DisplaySize.x, (int)imgui_context.io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(imgui_context.window);
    }

    {
        std::shared_lock lock(mutex_);
        abort = true;
    }
    if (client.is_running())
        client.stop();
    t.join();

    clean_up(imgui_context);

    return 0;
}
