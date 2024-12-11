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
#include <unordered_map>
#include <functional>
#include <vector>

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

                dockspace_id = ImGui::DockSpaceOverViewport(0,ImGui::GetMainViewport());
                dockspace_node = ImGui::DockBuilderGetNode(dockspace_id);

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
        ImGuiID dockspace_id;
        ImGuiDockNode* dockspace_node;

    private:
        GLFWwindow* window = nullptr;
        const char* glsl_version = "#version 130";

    // private:
    //     std::unordered_map<std::string,ImGui::ImGuiKey> key_maping = {
    //         {"tab",ImGuiKey_Tab},
    //         {"leftarrow",ImGuiKey_LeftArrow},
    //         {"rightarrow",ImGuiKey_RightArrow},
    //         {"uparrow",ImGuiKey_UpArrow},
    //         {"downarrow",ImGuiKey_DownArrow},
    //         {"pageup",ImGuiKey_PageUp},
    //         {"pagedown",ImGuiKey_PageDown},
    //         {"home",ImGuiKey_Home},
    //         {"end",ImGuiKey_End},
    //         {"insert",ImGuiKey_Insert},
    //         {"delete",ImGuiKey_Delete},
    //         {"backspace",ImGuiKey_Backspace},
    //         {"space",ImGuiKey_Space},
    //         {"enter",ImGuiKey_Enter},
    //         {"escape",ImGuiKey_Escape},
    //         {"apostrophe",ImGuiKey_Apostrophe},
    //         {"comma",ImGuiKey_Comma},
    //         {"minus",ImGuiKey_Minus},
    //         {"period",ImGuiKey_Period},
    //         {"slash",ImGuiKey_Slash},
    //         {"semicolon",ImGuiKey_Semicolon},
    //         {"equal",ImGuiKey_Equal},
    //         {"leftbracket",ImGuiKey_LeftBracket},
    //         {"backslash",ImGuiKey_Backslash},
    //         {"rightbracket",ImGuiKey_RightBracket},
    //         {"graveaccent",ImGuiKey_GraveAccent},
    //         {"capslock",ImGuiKey_CapsLock},
    //         {"scrolllock",ImGuiKey_ScrollLock},
    //         {"numlock",ImGuiKey_NumLock},
    //         {"printscreen",ImGuiKey_PrintScreen},
    //         {"pause",ImGuiKey_Pause},
    //         {"keypad0",ImGuiKey_Keypad0},
    //         {"keypad1",ImGuiKey_Keypad1},
    //         {"keypad2",ImGuiKey_Keypad2},
    //         {"keypad3",ImGuiKey_Keypad3},
    //         {"keypad4",ImGuiKey_Keypad4},
    //         {"keypad5",ImGuiKey_Keypad5},
    //         {"keypad6",ImGuiKey_Keypad6},
    //         {"keypad7",ImGuiKey_Keypad7},
    //         {"keypad8",ImGuiKey_Keypad8},
    //         {"keypad9",ImGuiKey_Keypad9},
    //         {"keypaddecimal",ImGuiKey_KeypadDecimal},
    //         {"keypaddivide",ImGuiKey_KeypadDivide},
    //         {"keypadmultiply",ImGuiKey_KeypadMultiply},
    //         {"keypadsubtract",ImGuiKey_KeypadSubtract},
    //         {"keypadadd",ImGuiKey_KeypadAdd},
    //         {"keypadenter",ImGuiKey_KeypadEnter},
    //         {"keypadequal",ImGuiKey_KeypadEqual},
    //         {"leftshift",ImGuiKey_LeftShift},
    //         {"leftctrl",ImGuiKey_LeftCtrl},
    //         {"leftalt",ImGuiKey_LeftAlt},
    //         {"leftsuper",ImGuiKey_LeftSuper},
    //         {"rightshift",ImGuiKey_RightShift},
    //         {"rightctrl",ImGuiKey_RightCtrl},
    //         {"rightalt",ImGuiKey_RightAlt},
    //         {"rightsuper",ImGuiKey_RightSuper},
    //         {"menu",ImGuiKey_Menu},
    //         {"0",ImGuiKey_0},
    //         {"1",ImGuiKey_1},
    //         {"2",ImGuiKey_2},
    //         {"3",ImGuiKey_3},
    //         {"4",ImGuiKey_4},
    //         {"5",ImGuiKey_5},
    //         {"6",ImGuiKey_6},
    //         {"7",ImGuiKey_7},
    //         {"8",ImGuiKey_8},
    //         {"9",ImGuiKey_9},
    //         {"a",ImGuiKey_A},
    //         {"b",ImGuiKey_B},
    //         {"c",ImGuiKey_C},
    //         {"d",ImGuiKey_D},
    //         {"e",ImGuiKey_E},
    //         {"f",ImGuiKey_F},
    //         {"g",ImGuiKey_G},
    //         {"h",ImGuiKey_H},
    //         {"i",ImGuiKey_I},
    //         {"j",ImGuiKey_J},
    //         {"k",ImGuiKey_K},
    //         {"l",ImGuiKey_L},
    //         {"m",ImGuiKey_M},
    //         {"n",ImGuiKey_N},
    //         {"o",ImGuiKey_O},
    //         {"p",ImGuiKey_P},
    //         {"q",ImGuiKey_Q},
    //         {"r",ImGuiKey_R},
    //         {"s",ImGuiKey_S},
    //         {"t",ImGuiKey_T},
    //         {"u",ImGuiKey_U},
    //         {"v",ImGuiKey_V},
    //         {"w",ImGuiKey_W},
    //         {"x",ImGuiKey_X},
    //         {"y",ImGuiKey_Y},
    //         {"z",ImGuiKey_Z},
    //         {"f1",ImGuiKey_F1},
    //         {"f2",ImGuiKey_F2},
    //         {"f3",ImGuiKey_F3},
    //         {"f4",ImGuiKey_F4},
    //         {"f5",ImGuiKey_F5},
    //         {"f6",ImGuiKey_F6},
    //         {"f7",ImGuiKey_F7},
    //         {"f8",ImGuiKey_F8},
    //         {"f9",ImGuiKey_F9},
    //         {"f10",ImGuiKey_F10},
    //         {"f11",ImGuiKey_F11},
    //         {"f12",ImGuiKey_F12},
    //         {"f13",ImGuiKey_F13},
    //         {"f14",ImGuiKey_F14},
    //         {"f15",ImGuiKey_F15},
    //         {"f16",ImGuiKey_F16},
    //         {"f17",ImGuiKey_F17},
    //         {"f18",ImGuiKey_F18},
    //         {"f19",ImGuiKey_F19},
    //         {"f20",ImGuiKey_F20},
    //         {"f21",ImGuiKey_F21},
    //         {"f22",ImGuiKey_F22},
    //         {"f23",ImGuiKey_F23},
    //         {"f24",ImGuiKey_F24}
    //     };
    };
}

#endif // IMGUI_WRAPPER_H
