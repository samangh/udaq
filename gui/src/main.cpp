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

struct AxisMinMax
{
    double minimum;
    double maximum;
};

struct FBGData
{
public:
    bool showPlot;
    bool autoScale = true;

    void add(udaq::devices::safibra::SensorReadout in)
    {
        using namespace udaq::common;

        if (size() == 0)
        {
            auto y_minmax = std::minmax_element(std::begin(in.readouts), std::end(in.readouts));
            m_wavelength_minmax.minimum = *y_minmax.first;
            m_wavelength_minmax.maximum = *y_minmax.second;

            m_time_minmax.minimum = in.time.front();
            m_time_minmax.maximum = in.time.back();
        }
        else
        {
            auto y_minmax = std::minmax_element(std::begin(in.readouts), std::end(in.readouts));
            double in_y_min = *y_minmax.first;
            double in_y_max = *y_minmax.second;

            m_wavelength_minmax.minimum = (in_y_min < m_wavelength_minmax.minimum) ? in_y_min : m_wavelength_minmax.minimum;
            m_wavelength_minmax.maximum = (in_y_max > m_wavelength_minmax.maximum) ? in_y_max : m_wavelength_minmax.maximum;

            m_time_minmax.maximum = in.time.back();
        }

        vector::append(m_wavelength, in.readouts);
        vector::append(m_time, in.time);

        for (size_t i = 0; i < in.time.size(); i++)
            writer.write(fmt::format("{},{}\n", in.time[i], in.readouts[i]));
    }

    const std::vector<double>& time() const { return m_time; }
    const std::vector<double>& wavelength() const { return m_wavelength; }
    AxisMinMax time_minmax() const
    {
        return m_time_minmax;
    }
    AxisMinMax twavelength_minmax() const
    {
        return m_wavelength_minmax;
    }
    size_t size() const { return m_time.size(); }
    void clear()
    {
        m_time.clear();
        m_wavelength.clear();
        m_time_minmax = AxisMinMax();
        m_wavelength_minmax = AxisMinMax();
    }
    udaq::common::file_writer writer;
private:
    std::vector<double> m_time;
    std::vector<double> m_wavelength;
    AxisMinMax m_time_minmax;
    AxisMinMax m_wavelength_minmax;
};

void add_fbg_data(const std::string folder, std::map<std::string, std::map<std::string, FBGData>>& data,
    const std::vector<udaq::devices::safibra::SensorReadout> &data_in, udaq::common::file_writer::error_cb_t error_c) {
    using namespace udaq::common;
    for (const auto& fbg_data_in : data_in)
    {
        auto& fbg = data[fbg_data_in.device_id][fbg_data_in.sensor_id];
        fbg.add(fbg_data_in);
        if (!fbg.writer.is_running())
            fbg.writer.start(folder + "\\"+ fbg_data_in.sensor_id, error_c, []() {}, []() {});
    }
    
}

#ifdef WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int, char**)
{
    std::vector<udaq::common::file_writer> writers;

    std::string path = std::filesystem::current_path().string();

    auto imgui_context=initialise();
    //ImPlot::GetStyle().AntiAliasedLines = true;

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::shared_mutex mutex_;

    std::map<std::string, std::map<std::string, FBGData>> data;

    // Main loop
    bool done = false;

    std::atomic<bool> error_received = false;
    std::atomic<bool> client_connected = false;
    std::atomic<bool> listening = false;
    std::string error_message;

    int port = 5555;
    unsigned int downsample_points = 1000;

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

    auto on_file_error = [&](const std::string message) {
        error_received = true;
        error_message = message;
    };

    auto on_data_available =
        [&](std::vector<udaq::devices::safibra::SensorReadout> data_in) {
            std::unique_lock lock(mutex_);
            add_fbg_data(path, data, data_in, on_file_error);
    };



    //std::thread t(do_work, std::ref(xs), std::ref(y), std::ref(abort), std::ref(mutex_));
    auto client = udaq::devices::safibra::SigprogServer(
        on_error, on_client_connected, on_client_disconnected,
        on_server_started, on_server_stopped, on_data_available);

    ImGui::FileBrowser fileDialog(ImGuiFileBrowserFlags_SelectDirectory|ImGuiFileBrowserFlags_CreateNewDir|ImGuiFileBrowserFlags_CloseOnEsc);

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

        ImGui::Begin("Interrogator Client",NULL, ImGuiWindowFlags_AlwaysAutoResize);
        {
            udaq::helpers::imgui::InputText("Save Directory", path, 0);
            if (ImGui::Button("Browse ..."))
            {
                //fileDialog.SetTitle("title");
                //fileDialog.SetTypeFilters({ ".h", ".cpp" });
                fileDialog.SetPwd(path);
                fileDialog.Open();
            }
            ImGui::Separator();

            disable_item(client.is_running(), [&]() {
                ImGui::InputInt("Port", &port);
            });

            disable_item(client.is_running(), [&](){
                if (ImGui::Button("Listen"))
                {
                    client.start(port);
                }
            });
            ImGui::SameLine();
            disable_item(!client.is_running(), [&]() {
                if (ImGui::Button("Stop Listening"))
                    client.stop();
            });

            ImGui::Separator();

            ImGui::RadioButton("Listening", listening);
            ImGui::RadioButton("Interrogator connected", client_connected);


            {
                std::shared_lock lock(mutex_);
                if (data.size() > 0)
                {
                   bool is_client_running = client.is_running();
                    ImGui::Separator();
                    ImGui::Text("Interrogators:");
                    for (auto& [device, sensors] : data)
                        if (ImGui::TreeNode(device.c_str()))
                        {
                            for (auto& [sensor, fbg] : sensors)
                            {
                                ///ImGui::Checkbox(fmt::format("Plot {}", sensor).c_str(), &fbg.showPlot);
                                ImGui::Text(fmt::format("{}:", sensor).c_str());
                                ImGui::SameLine();
                                ImGui::RadioButton("Recording",  is_client_running && fbg.writer.is_running());
                                ImGui::SameLine();
                                if (ImGui::Button(fmt::format("Show plot...##{}", sensor).c_str()))
                                    fbg.showPlot=true;
                            }

                            ImGui::TreePop();
                        }


                    if (ImGui::Button("Clear all data"))
                        for (auto& [device, sensors] : data)
                            for (auto& [sensor, fbg] : sensors)
                                fbg.clear();
                }
                 
            }

        }
        ImGui::End();

        fileDialog.Display();
        if(fileDialog.HasSelected())
        {
            path=fileDialog.GetSelected().string();
            fileDialog.ClearSelected();
        }

        if (data.size() >0){
            std::shared_lock lock(mutex_);

            for (auto& [device, sensors] : data)

                for (auto& [sensor_id, fbg] : sensors)
                    if (fbg.showPlot && fbg.time().size() > 0)
                {

                    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(10240, 10240));
                    ImGui::Begin(sensor_id.c_str(), &fbg.showPlot);

                    ImGui::Checkbox("Autoscale", &fbg.autoScale);
                    if (fbg.size() > 100)
                    {
                        ImGui::SameLine();
                        ImGui::Text("Average rate: %.1f Hz", 100.0 / (fbg.time().back() - fbg.time()[fbg.size()-100] ));
                    }

                    ImPlot::SetNextPlotLimits(fbg.time_minmax().minimum, fbg.time_minmax().maximum,
                        fbg.twavelength_minmax().minimum, fbg.twavelength_minmax().maximum,
                        fbg.autoScale ? ImGuiCond_::ImGuiCond_Always : ImGuiCond_::ImGuiCond_Once);
                    if (ImPlot::BeginPlot(sensor_id.c_str(), NULL, NULL, ImVec2(-1, -1), ImGuiCond_::ImGuiCond_Always, ImPlotAxisFlags_Time)) {

                        if (fbg.size() > downsample_points)
                        {
                            using namespace udaq::common;
                            auto start_index = vector::upper_bound_index(fbg.time(), ImPlot::GetPlotLimits().X.Min);
                            auto end_index = vector::lower_bound_index(fbg.time(), ImPlot::GetPlotLimits().X.Max);

                            auto seperation = end_index - start_index;
                            if (seperation < downsample_points)
                                ImPlot::PlotLine(sensor_id.c_str(), &fbg.time()[start_index], &fbg.wavelength()[start_index], seperation);
                            else
                            {
                                int stride_length = (int)ceil(seperation / (double)downsample_points);
                                int count = floor(seperation / (double)stride_length);
                                ImPlot::PlotLine(sensor_id.c_str(), &fbg.time()[start_index], &fbg.wavelength()[start_index], count, 0, sizeof(double)*stride_length);
                            }
                        }
                        else
                        {
                            ImPlot::PlotLine(sensor_id.c_str(), &fbg.time()[0], &fbg.wavelength()[0], (int)(fbg.size()));
                        }

                        ImPlot::EndPlot();
                    }
                    ImGui::End();
                }
         }

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
                    for (auto& [device, sensors] : data)
                        for (auto& [sensor_id, fbg] : sensors)
                        {
                            if (fbg.writer.is_running())
                                fbg.writer.stop();
                        }
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


    if (client.is_running())
        client.stop();

    clean_up(imgui_context);

    return 0;
}
