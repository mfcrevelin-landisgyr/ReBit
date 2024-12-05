#ifndef IMGUI_WRAPPER_H
#define IMGUI_WRAPPER_H

#include "imgui.h"
#include <imgui_internal.h>
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

namespace ImGui {
    class Wrapper {
    public:
        Wrapper() {}
        virtual ~Wrapper() { Cleanup(); }

    public:
        virtual bool Create() { return true; }
        virtual bool Update() { return true; }

    public:
        bool Construct(int width = 1280, int height = 720) {
            glfwSetErrorCallback(glfw_error_callback);
            if (!glfwInit())
                return false;

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            window = glfwCreateWindow(width, height, title, nullptr, nullptr);
            if (!window)
                return false;

            glfwMakeContextCurrent(window);
            glfwSwapInterval(1);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            if (!Create())
                return false;

            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init(glsl_version);

            return true;
        }

        void Start() {
            if (!window)
                return;

            bool dock_layout_setup = true;
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
                    ImGui_ImplGlfw_Sleep(10);
                    continue;
                }

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                ImGui::DockSpaceOverViewport(0,ImGui::GetMainViewport());

                if (!Update())
                    break;

                ImGui::Render();
                int display_w, display_h;
                glfwGetFramebufferSize(window, &display_w, &display_h);
                glViewport(0, 0, display_w, display_h);
                // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                // glClear(GL_COLOR_BUFFER_BIT);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                ImGuiIO& io = ImGui::GetIO();
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    GLFWwindow* backup_current_context = glfwGetCurrentContext();
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    glfwMakeContextCurrent(backup_current_context);
                }

                glfwSwapBuffers(window);
            }
        }

    private:
        static void glfw_error_callback(int error, const char* description) {
            std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        }

        void Cleanup() {
            if (window) {
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
