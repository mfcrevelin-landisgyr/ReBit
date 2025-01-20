#ifndef REBIT_H
#define REBIT_H

#include "ImguiWrapper.h"
#include "tinyfiledialogs.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <pybind11/stl.h>

#include <string_view>
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <memory>
#include <vector>
#include <chrono>
#include <queue>
#include <string>
#include <thread>
#include <stack>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace fs = std::filesystem;
namespace py = pybind11;

#include "FileHandler.h"

class ReBit : public ImGui::Wrapper {
public:
    ReBit() {
        title = "ReBit";
    }

    bool Create() override {
        ImGui::StyleColorsLight();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF("JetBrainsMonoNL-Regular.ttf", 18.0f);

        return true;
    }

    bool Update() override {

        ShowMainMenuBar();
        ShowFileWindows();
        ShowProjectPannel();
        ShowPopups();
        ManageFiles();

        ProcessPeripheralInputs();

        current_window = nullptr;
        return true;
    }

private:

    void ProcessPeripheralInputs() {

        if (current_window){

            switch(std::hash<std::string>{}(current_window->Name)) {
            case 0xff72950c934d8a08: // Project Panel
                
                if (ImGui::IsKeyPressed(ImGuiKey_Enter) && !selected_files.empty()) {
                    for (const auto& selected_path : selected_files)
                        to_open.insert(selected_path);
                    selected_files.clear();
                }

                break;
            case 0xbfc58a6f1067a1ed: // Error Panel
                break;
            case 0x2aefb7fa9d57a9ed: // Log Panel
                break;
            case 0x58e72e8c5c91d068: // Modules Panel
                break;
            }
        }

        
        if (ImGui::GetIO().KeyCtrl) {
            if (ImGui::IsKeyPressed(ImGuiKey_O) && !browsing_dialog){
                browsing_dialog=true;
                std::thread([this]() {
                    const char* selected_path = tinyfd_selectFolderDialog("Select a folder",project_path.string().c_str());

                    if (selected_path != NULL && fs::is_directory(selected_path))
                        SetProjectPath(selected_path);
                    else {

                        const std::string t = "Error";
                        const std::string m = "The chosen path must be a directory.";
                        const std::vector<std::pair<std::string, std::function<void()>>>& b = {
                            std::make_pair("OK", []() {})
                        };

                        popups.emplace_back(t,m,b);
                    }



                    browsing_dialog = false;
                }).detach();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_R))
                PopulateDirectoryFilesList();
        }

    }

    void ManageFiles() {
        for (auto& path : to_open) {
            if (opened_files.contains(path))
                ImGui::SetWindowFocus(path.filename().string().c_str());
            else
                opened_files.insert(path);
        }
        for (auto& path : to_close)
            opened_files.erase(path);
        to_open.clear();
        to_close.clear();
    }

private:

    void ShowMainMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Project")) {
                if (ImGui::MenuItem("Save All")) {}
                if (ImGui::MenuItem("Render All")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Project", nullptr, &show_project_pannel);
                ImGui::MenuItem("Error", nullptr, &show_error_pannel);
                ImGui::MenuItem("Log", nullptr, &show_log_pannel);
                ImGui::MenuItem("Modules", nullptr, &show_modules_pannel);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Render")) {
                if (ImGui::MenuItem("Script", nullptr, false, renderable_script)) { /* Handle script rendering */ }
                if (ImGui::MenuItem("Project")) { /* Handle project rendering */ }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void ShowProjectPannel() {
        static fs::path last_clicked_file;
        static double last_click_time = 0.0;
        constexpr double double_click_dt = 0.3;

        if (show_project_pannel) {
            ImGui::Begin("Project", &show_project_pannel, ImGuiWindowFlags_NoCollapse);
            
            if (ImGui::BeginMenuBar()) {
                if (ImGui::MenuItem("Saver")) {}
                ImGui::EndMenuBar();
            }
                
                if (ImGui::IsWindowFocused())
                    current_window = ImGui::GetCurrentWindow();

                for (const auto& [path,file_handler] : project_files) {
                    bool is_selected = selected_files.contains(path);

                    if (ImGui::Selectable(path.filename().string().c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                        if (ImGui::GetIO().KeyCtrl) {
                            if (is_selected)
                                selected_files.erase(path);
                            else
                                selected_files.insert(path);
                        } else {
                            selected_files.clear();
                            selected_files.insert(path);

                            double current_time = ImGui::GetTime();
                            if (path == last_clicked_file && (current_time - last_click_time) < double_click_dt) {
                                to_open.insert(path);
                                selected_files.clear();
                            }

                            last_clicked_file = path;
                            last_click_time = current_time;
                        }
                    }
                }

            ImGui::End();
        }

    }

    void ShowFileWindows(){

        std::set<fs::path> to_close;
        for (const auto& file_path : opened_files) {
            std::string window_name = file_path.filename().string();
            bool is_open = true;

            ImGui::SetNextWindowDockID(default_dock_id, ImGuiCond_FirstUseEver);
            ImGui::Begin(window_name.c_str(), &is_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
            ImGui::Text("Displaying content for file: %s", file_path.filename().string().c_str());
            ImGui::End();

            if (!is_open)
                to_close.insert(file_path);
        }

        for (const auto& file_path : to_close) 
            opened_files.erase(file_path);
    }

    void ShowPopups() {
        for (size_t i = 0; i < popups.size(); ) {
            PopupHandler& popup = popups[i];

            ImGui::OpenPopup(popup.title.c_str());
            
            if (ImGui::BeginPopupModal(popup.title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("%s", popup.message.c_str());
                
                for (auto it = popup.buttons.begin(); it != popup.buttons.end(); ++it) {
                    auto& [label, action] = *it;
                    if (ImGui::Button(label.c_str())) {
                        action(); // Call the action
                        ImGui::CloseCurrentPopup();
                    }
                    if (std::next(it) != popup.buttons.end())
                        ImGui::SameLine();
                }
                ImGui::EndPopup();
            }

            if (!ImGui::IsPopupOpen(popup.title.c_str()))
                popups.erase(popups.begin() + i);
            else
                ++i;
        }
    }


private:

    void SetProjectPath(const fs::path new_path) {
        if (fs::exists(new_path) && fs::is_directory(new_path)) {

            if (!HasRWPermission(new_path)) {
                const std::string t = "Permission Error";
                const std::string m = "The chosen path could not be opened. \nMissing RW permissions.";
                const std::vector<std::pair<std::string, std::function<void()>>>& b = {
                    std::make_pair("OK", []() {})
                };

                popups.emplace_back(t,m,b);
                return;
            }

            if (!modified_files.empty()) {
                const std::string t = "Unsaved Changes";
                const std::string m = "You have unsaved changes. Do you want to save them before switching the project directories?";
                const std::vector<std::pair<std::string, std::function<void()>>>& b = {
                    std::make_pair("Save & Switch", [this, new_path]() { 
                        /* Save logic */ 
                        project_path = new_path;
                        PopulateDirectoryFilesList();
                    }),
                    std::make_pair("Don't Save", [this, new_path]() {
                        project_path = new_path;
                        PopulateDirectoryFilesList();
                    }),
                    std::make_pair("Cancel", []() {})
                };

                popups.emplace_back(t,m,b);
                return;
            }

            project_path = new_path;
            PopulateDirectoryFilesList();

        } else {
            const std::string t = "Invalid Path";
            const std::string m = "The chosen path does not exist or is not a directory.";
            const std::vector<std::pair<std::string, std::function<void()>>>& b = {
                std::make_pair("OK", []() {})
            };
            popups.emplace_back(t,m,b);
        }
    }

    void PopulateDirectoryFilesList() {

        project_files.clear();

        for (const auto& entry : fs::directory_iterator(project_path)) {
            const auto& path = entry.path();

            if (path.filename().string().starts_with(".") || fs::is_directory(path))
                continue;

            project_files[path] = std::make_unique<FileHandler>(path, output_directory);

        }
    }

    bool HasRWPermission(const fs::path& path) {
        auto perms = fs::status(path).permissions();
        constexpr auto read_write_perms = 
            fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read |
            fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write;
        return (perms & read_write_perms) != fs::perms::none;
    }

private:
    struct PopupHandler {
        std::string title;
        std::string message;
        std::vector<std::pair<std::string, std::function<void()>>> buttons;
        
        PopupHandler(
            const std::string& t,
            const std::string& m,
            const std::vector<std::pair<std::string, std::function<void()>>>& b
        ) : title(t), message(m), buttons(b) {}
    };

    std::vector<PopupHandler> popups;

private:

private:
    std::set<fs::path> opened_files;
    std::unordered_set<fs::path> to_open;
    std::unordered_set<fs::path> to_close;
    std::unordered_set<fs::path> modified_files;
    bool renderable_script = false;

private:
    fs::path project_path;
    fs::path output_directory;
    std::unordered_map<fs::path,std::unique_ptr<FileHandler>> project_files;
    std::unordered_set<fs::path> selected_files;
    bool browsing_dialog = false;

private:
    ImGuiWindow* current_window = nullptr;

    bool show_project_pannel = false;
    ImGuiWindow* project_pannel_ptr = nullptr;

    bool show_error_pannel = false;
    ImGuiWindow* error_pannel_ptr = nullptr;

    bool show_log_pannel = false;
    ImGuiWindow* log_pannel_ptr = nullptr;

    bool show_modules_pannel = false;
    ImGuiWindow* modules_pannel_ptr = nullptr;


private:
    ImGuiID default_dock_id;
};



    // void CumputeLayout() {
    //     default_dock_id = dockspace_id;
    //     if (show_project_pannel){
    //         ImGuiWindow* directory_panel_ptr = ImGui::FindWindowByName("Project");
    //         ImGuiID directory_panel_id = directory_panel_ptr->ID;

    //         bool panel_docked_main = false;
    //         bool panel_docked_child = false;

    //         for (ImGuiWindow* window : dockspace_node->Windows) {
    //             if (panel_docked_main){
    //                 ImGuiID left_child = 0, right_child = 0;
    //                 ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &left_child, &right_child);
    //                 ImGui::DockBuilderDockWindow("Directory", left_child);
    //                 default_dock_id = right_child;  
    //                 return;
    //             }
    //             panel_docked_main |= window->ID == directory_panel_id;
    //         }

    //         if (dockspace_node->ChildNodes[1]) { // If it has Child[1], then it has Child[0]
                
    //             for (ImGuiWindow* window : dockspace_node->ChildNodes[1]->Windows) {
    //                 if (panel_docked_child) {
    //                     default_dock_id = dockspace_node->ChildNodes[0]->ID;
    //                     return;
    //                 }
    //                 panel_docked_child |= window->ID == directory_panel_id;
    //             }

    //             default_dock_id = dockspace_node->ChildNodes[1]->ID;
    //         }
    //     }
    // }

#endif // REBIT_H
