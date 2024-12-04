#ifndef IMGUI_WRAPPER_H
#define IMGUI_WRAPPER_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GL_SILENCE_DEPRECATION

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

#include <iostream>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

namespace ImGui
{
    class Wrapper
    {
    public:
        Wrapper() {}
        virtual ~Wrapper() { Cleanup(); }

        // Main entry point
        bool Construct(int width = 1280, int height = 720)
        {
            // Set GLFW error callback
            glfwSetErrorCallback(glfw_error_callback);
            if (!glfwInit())
                return false;

            // Configure OpenGL and GLFW
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            window = glfwCreateWindow(width, height, title, nullptr, nullptr);
            if (!window)
                return false;

            glfwMakeContextCurrent(window);
            glfwSwapInterval(1); // Enable vsync

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

            // User-defined settings
            if (!Create())
                return false;

            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init(glsl_version);

            return true;
        }

        // Starts the main loop
        void Start()
        {
            if (!window)
                return;

            while (!glfwWindowShouldClose(window))
            {
                // Poll and handle events
                glfwPollEvents();
                if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
                {
                    ImGui_ImplGlfw_Sleep(10);
                    continue;
                }

                // Start ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                // Mandatory dockspace covering the entire viewport
                ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

                // User-defined rendering
                if (!Update())
                    break;

                // Rendering
                ImGui::Render();
                int display_w, display_h;
                glfwGetFramebufferSize(window, &display_w, &display_h);
                glViewport(0, 0, display_w, display_h);
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT); // Optional, can be removed for no clearing
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // Update and render additional platform windows
                ImGuiIO& io = ImGui::GetIO();
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                {
                    GLFWwindow* backup_current_context = glfwGetCurrentContext();
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    glfwMakeContextCurrent(backup_current_context);
                }

                glfwSwapBuffers(window);
            }
        }

        // Overridable methods for user customization
        virtual bool Create() { return true; } // Setup custom ImGui settings
        virtual bool Update() { return true; } // Main application logic

    private:

        static void glfw_error_callback(int error, const char* description)
        {
            std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        }

        void Cleanup()
        {
            if (window)
            {
                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();
                glfwDestroyWindow(window);
                glfwTerminate();
                window = nullptr;
            }
        }
    public:
        const char* title = "ImGui Application";
    private:
        GLFWwindow* window = nullptr;
        const char* glsl_version = "#version 130";
    };
}

#endif // IMGUI_WRAPPER_H
