#include <iostream>
#include <atomic>
#include <functional>
#include <imgui-wrapper.h>
#include <implot.h>

#include "karla-font.h"


class MyContext {

public:
    ImGuiIO io;
    SDL_Window *window;
    void initialise(const std::string& title){
        cleaned_up =false;

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
        window = SDL_CreateWindow(title.c_str(),  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
        gl_context = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, gl_context);

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
        io = ImGui::GetIO();
        (void)io;

        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
            Karla_compressed_data, karla_compressed_size, 14);
        io.Fonts->Build();

        ImGui::StyleColorsDark();
        ImGui::GetStyle().WindowRounding = 5.0f;

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }
    void start_gui_loop(std::function<void(void)> func)
    {
       gui_done = false;

       while (!gui_done)
       {
           SDL_Event event;
           while (SDL_PollEvent(&event))
           {
               ImGui_ImplSDL2_ProcessEvent(&event);
               if (event.type == SDL_QUIT)
                   gui_done = true;
               if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                   gui_done = true;
           }

           // Start the Dear ImGui frame
           ImGui_ImplOpenGL3_NewFrame();
           ImGui_ImplSDL2_NewFrame(window);

           func();

           // Rendering
           ImGui::Render();
           glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
           glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
           glClear(GL_COLOR_BUFFER_BIT);
           ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
           SDL_GL_SwapWindow(window);
       }

    }

    /* Stops the GUI loop. This is not thread safe, it should only be called within the function passed to start_gui_loop().*/
    void stop_gui_loop()
    {
        gui_done=true;
    }

    void cleanup() {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();

        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();

        cleaned_up = true;
    }

    ~MyContext(){
        if (!cleaned_up)
            cleanup();
    }

private:
    bool gui_done;
    std::atomic<bool> cleaned_up;
    SDL_GLContext gl_context;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

};
