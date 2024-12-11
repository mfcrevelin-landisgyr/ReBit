#ifndef REBIT_H
#define REBIT_H

#include "ImguiWrapper.h"
#include "tinyfiledialogs.h"

#include <string_view>
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <stack>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace fs = std::filesystem;

#include "TableFile.h"

class ReBit : public ImGui::Wrapper {
public:
    ReBit() {
        title = "ReBit";
    }

    bool Create() override {
        ImGui::StyleColorsLight();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF("JetBrainsMonoNL-Regular.ttf", 18.0f);

        strncpy_s(project_path, project_path.string().c_str(), sizeof(project_path));

        PopulateDirectoryList();

        return true;
    }

    bool Update() override {

        ProcessInput();

        RenderMainMenuBar();
        ShowProjectPannel();

        ManageFiles();

        // CumputeLayout();

        RenderFileWindows();

        return true;
    }

private:

    void ProcessInput() {
        ImGuiWindow* focused_window = ImGui::GetCurrentContext()->NavWindow;

        if (show_project_pannel && focused_window == ImGui::FindWindowByName("Project Panel")){
            if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                for (const auto& selected_path : selected_files)
                    to_open.push(selected_path);
                selected_files.clear();
            }
        }

        if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O))
                BrowseDialog();
    }

    void CumputeLayout() {
        default_dock_id = dockspace_id;
        if (display_directory_panel){
            ImGuiID directory_panel_id = ImGui::FindWindowByName("Directory Panel")->ID;

            bool panel_docked_main = false;
            bool panel_docked_child = false;

            for (ImGuiWindow* window : dockspace_node->Windows) {
                if (panel_docked_main){
                    ImGuiID left_child = 0, right_child = 0;
                    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, &left_child, &right_child);
                    ImGui::DockBuilderDockWindow("Directory Panel", left_child);
                    default_dock_id = right_child;  
                    return;
                }
                panel_docked_main |= window->ID == directory_panel_id;
            }

            if (dockspace_node->ChildNodes[1]) { // If it has Child[1], then it has Child[0]
                
                for (ImGuiWindow* window : dockspace_node->ChildNodes[1]->Windows) {
                    if (panel_docked_child) {
                        default_dock_id = dockspace_node->ChildNodes[0]->ID;
                        return;
                    }
                    panel_docked_child |= window->ID == directory_panel_id;
                }

                default_dock_id = dockspace_node->ChildNodes[1]->ID;
            }
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

    void RenderMainMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save")) { /* Handle save */ }
                if (ImGui::MenuItem("Save All")) { /* Handle save all */ }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Project Panel", nullptr, &show_project_pannel);
                ImGui::MenuItem("Error Panel", nullptr, &show_error_pannel);
                ImGui::MenuItem("Log Panel", nullptr, &show_log_pannel);
                ImGui::MenuItem("Modules Panel", nullptr, &show_modules_pannel);
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
        static float last_click_time = 0.0f;
        constexpr float double_click_dt = 0.3f;

        if (ImGui::Begin("Project Panel", &show_project_pannel)) {

            ImGui::BeginChild("ScrollArea", ImGui::GetContentRegionAvail() * 0.95f, ImGuiChildFlags_None, ImGuiWindowFlags_None);


                for (const auto& path : project_files) {
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

                            float current_time = ImGui::GetTime();
                            if (path == last_clicked_file && (current_time - last_click_time) < double_click_dt) {
                                to_open.push(path);
                                selected_files.clear();
                            }

                        }

                        last_clicked_file = path;
                        last_click_time = current_time;
                    }
                }

            ImGui::EndChild();

        ImGui::End();
        }
    }

    void RenderFileWindows(){

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

private:

    void BrowseDialog() {
        if (!browsing_dialog){
            browsing_dialog=true;
            std::thread([this]() {
                const char* selected_path = tinyfd_selectFolderDialog(
                    "Select a folder",
                    project_path.string().c_str()
                );

                if (selected_path != NULL && fs::is_directory(selected_path))
                    SetProjectPath(selected_path);

                browsing_dialog = false;
            }).detach();
        }
    }

    void SetProjectPath(const fs::path new_path) {

        if (!HasWRPermission(new_path)) {
            ImGui::OpenPopup("Permission Error");
            if (ImGui::BeginPopupModal("Permission Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("The chosen path could not be opened due to lack of permissions.");
                if (ImGui::Button("OK"))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            return;
        }

        if (!modified_files.empty()) {
            ImGui::OpenPopup("Unsaved Changes");
            if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("You have unsaved changes. Do you want to save them before changing the project path?");
                if (ImGui::Button("Save")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Don't Save")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    return;
                }
                ImGui::EndPopup();
            }
        }

        if (fs::exists(new_path) && fs::is_directory(new_path)) {
            backwards_directory_history.push(project_path);
            project_path = new_path;
            while(!forwards_directory_history.empty())
                forwards_directory_history.pop();
            PopulateDirectoryList();
        }

        strncpy_s(project_path, project_path.string().c_str(), sizeof(project_path));
    }

    void PopulateDirectoryList() {

        project_files.clear();

        for (auto& entry : fs::directory_iterator(project_path)) {
            const auto& path = entry.path();

            if (path.filename().string().starts_with(".") || fs::is_directory(path))
                continue;

            TableFile* table_file = ParseFile(path);

            if (table_file != nullptr){
                project_files[path] = nullptr;
                delete table_file;
            }


        }
    }

    TableFile* ParseFile(const fs::path& path) {

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return nullptr;

        char ch;
        while (file.get(ch)) {
            if (!isprint(static_cast<unsigned char>(ch)) && !isspace(static_cast<unsigned char>(ch))) {
                file.close();
                return nullptr;
            }
        }

        file.close();
        return new TableFile(path);
    }

    bool HasWRPermission(const fs::path& path) {
        auto perms = fs::status(path).permissions();
        constexpr auto read_write_perms = 
            fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read |
            fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write;
        return (perms & read_write_perms) != fs::perms::none;
    }

private:
    std::set<fs::path> opened_files;
    std::unordered_set<fs::path> to_open;
    std::unordered_set<fs::path> to_close;
    std::unordered_set<fs::path> modified_files = {"C:"};

private:
    char project_path[512];
    fs::path project_path = "C:\\Users\\crevelim\\Desktop";
    std::unordered_map<fs::path,TableFile*> project_files;
    std::unordered_set<fs::path> selected_files;
    bool browsing_dialog = false;

private:
    bool show_project_pannel = false;
    bool show_error_pannel = false;
    bool show_log_pannel = false;
    bool show_modules_pannel = false;

private:
    ImGuiID default_dock_id;
};

#endif // REBIT_H
