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
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF("JetBrainsMonoNL-Regular.ttf", 18.0f);

        SetProjectPath("C:\\Users\\crevelim\\Desktop");

        return true;
    }

    bool Update() override {

        ProcessInputs();

        ShowMainMenuBar();
        ShowFileWindows();
        ShowProjectPannel();
        ShowPopups();
        ManageFiles();

        return true;
    }

private:

    void ProcessInputs() {
        if (show_project_pannel){
            if (ImGui::IsKeyPressed(ImGuiKey_Enter) && !selected_files.empty()) {
                std::cout << "Enter" << std::endl;
                for (const auto& selected_path : selected_files)
                    to_open.insert(selected_path);
                selected_files.clear();
            }
        }

        if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O))
                BrowseDialog();
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
        static double last_click_time = 0.0;
        constexpr double double_click_dt = 0.3;

        if (show_project_pannel) {
            ImGui::Begin("Project Panel", &show_project_pannel, ImGuiWindowFlags_NoCollapse);

                for (const auto& [path,table_file] : project_files) {
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
            Popup& popup = popups[i];

            ImGui::OpenPopup(popup.title.c_str());
            
            if (ImGui::BeginPopupModal(popup.title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("%s", popup.message.c_str());
                
                for (auto it = popup.buttons.begin(); it != popup.buttons.end(); ++it) {
                    auto& [label, action] = *it;
                    if (ImGui::Button(label.c_str())) {
                        action(); // Call the action
                        ImGui::CloseCurrentPopup(); // Close the popup
                    }
                    // Add buttons on the same line if not the last button
                    if (std::next(it) != popup.buttons.end()) {
                        ImGui::SameLine();
                    }
                }
                ImGui::EndPopup();
            }

            if (!ImGui::IsPopupOpen(popup.title.c_str())) {
                popups.erase(popups.begin() + i);
            } else {
                ++i;
            }
        }
    }


private:

    void BrowseDialog() {
        if (!browsing_dialog){
            browsing_dialog=true;
            std::thread([this]() {
                const char* selected_path = tinyfd_selectFolderDialog("Select a folder",project_path.string().c_str());

                if (selected_path != NULL && fs::is_directory(selected_path))
                    SetProjectPath(selected_path);

                browsing_dialog = false;
            }).detach();
        }
    }

    void SetProjectPath(const fs::path new_path) {
        if (fs::exists(new_path) && fs::is_directory(new_path)) {

            if (!HasWRPermission(new_path)) {
                const std::string t = "Permission Error";
                const std::string m = "The chosen path could not be opened due to lack of permissions.";
                const std::vector<std::pair<std::string, std::function<void()>>>& b = {
                    std::make_pair("OK", []() {})
                };

                popups.emplace_back(t,m,b);
                return;
            }

            if (!modified_files.empty()) {
                const std::string t = "Unsaved Changes";
                const std::string m = "You have unsaved changes. Do you want to save them before changing the project directory?";
                const std::vector<std::pair<std::string, std::function<void()>>>& b = {
                    std::make_pair("Save", [this, new_path]() { 
                        /* Save logic */ 
                        project_path = new_path;
                        PopulateDirectoryList();
                    }),
                    std::make_pair("Don't Save", [this, new_path]() {
                        project_path = new_path;
                        PopulateDirectoryList();
                    }),
                    std::make_pair("Cancel", []() {})
                };

                popups.emplace_back(t,m,b);
                return;
            }

            project_path = new_path;
            PopulateDirectoryList();

        } else {
            const std::string t = "Invalid Path";
            const std::string m = "The chosen path does not exist or is not a directory.";
            const std::vector<std::pair<std::string, std::function<void()>>>& b = {
                std::make_pair("OK", []() {})
            };
            popups.emplace_back(t,m,b);
        }
    }

    void PopulateDirectoryList() {

        project_files.clear();

        for (const auto& entry : fs::directory_iterator(project_path)) {
            const auto& path = entry.path();

            if (path.filename().string().starts_with(".") || fs::is_directory(path))
                continue;

            TableFile* table_file = (TableFile*)1;//ParseFile(path);

            if (table_file != nullptr){
                project_files[path] = nullptr;
                // delete table_file;
            }

        }
    }

    bool HasWRPermission(const fs::path& path) {
        auto perms = fs::status(path).permissions();
        constexpr auto read_write_perms = 
            fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read |
            fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write;
        return (perms & read_write_perms) != fs::perms::none;
    }

private:
    struct Popup {
        std::string title;
        std::string message;
        std::vector<std::pair<std::string, std::function<void()>>> buttons;
        
        Popup(
            const std::string& t,
            const std::string& m,
            const std::vector<std::pair<std::string, std::function<void()>>>& b
        ) : title(t), message(m), buttons(b) {}
    };

    std::vector<Popup> popups;

private:
    std::set<fs::path> opened_files;
    std::unordered_set<fs::path> to_open;
    std::unordered_set<fs::path> to_close;
    std::unordered_set<fs::path> modified_files = {"C:"};
    bool renderable_script = false;

private:
    fs::path project_path;
    std::unordered_map<fs::path,TableFile*> project_files;
    std::unordered_set<fs::path> selected_files;
    bool browsing_dialog = false;

private:
    ImGuiWindow* project_pannel_ptr = nullptr;
    bool show_project_pannel = false;
    ImGuiWindow* error_pannel_ptr = nullptr;
    bool show_error_pannel = false;
    ImGuiWindow* log_pannel_ptr = nullptr;
    bool show_log_pannel = false;
    ImGuiWindow* modules_pannel_ptr = nullptr;
    bool show_modules_pannel = false;

private:
    ImGuiID default_dock_id;
};

#endif // REBIT_H
