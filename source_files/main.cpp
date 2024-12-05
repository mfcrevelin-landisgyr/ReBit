#include "imgui_wrapper.h"
#include "tinyfiledialogs.h"

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <cstdint>
#include <stack>
#include <map>

namespace fs = std::filesystem;

class ReBit : public ImGui::Wrapper
{
public:
    ReBit() {
        title = "ReBit";
    }

    bool Create() override {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontFromFileTTF("IBMPlexMono-Regular.ttf", 18.0f);

        std::strncpy(path_buffer, current_directory.string().c_str(), sizeof(path_buffer));

        return true;
    }

    bool Update() override {
        ImGui::ShowDemoWindow();
        directory_panel();
        tabs_panel();
        return true;
    }

private:
    void directory_panel(){
        ImGui::Begin("Project Panel",nullptr,ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);// ImGuiWindowFlags_NoMove

            ImGui::BeginGroup();
                if (ImGui::Button("<"))
                    GoBack();
            ImGui::SameLine();
                if (ImGui::Button("^"))
                    GoUp();
            ImGui::SameLine();
                if (ImGui::Button(">"))
                    GoForward();
            ImGui::SameLine();
                if (ImGui::InputText("##", path_buffer, sizeof(path_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    SetCurrentDirectory(path_buffer);
            ImGui::SameLine();
                if (ImGui::Button("Browse"))
                    BrowseDialog();
            ImGui::EndGroup();

            ImGui::Spacing();

            ImGui::BeginChild("ChildL", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, ImGuiWindowFlags_None);
                if (fs::is_directory(current_directory))
                    list_directory_content();
            ImGui::EndChild();
        
        ImGui::End();

    }

    void tabs_panel(){
        ImGui::Begin("Files Panel",nullptr,ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
            // if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None)) {
            //     if (ImGui::BeginTabItem("Avocado")) {
            //         ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
            //         ImGui::EndTabItem();
            //     }
            //     if (ImGui::BeginTabItem("Broccoli")) {
            //         ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
            //         ImGui::EndTabItem();
            //     }
            //     if (ImGui::BeginTabItem("Cucumber")) {
            //         ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
            //         ImGui::EndTabItem();
            //     }
            //     ImGui::EndTabBar();
            // }



            static std::vector<int> active_tabs;
            static int next_tab_id = 0;
            if (next_tab_id == 0) // Initialize with some default tabs
                for (int i = 0; i < 3; i++)
                    active_tabs.push_back(next_tab_id++);

            // TabItemButton() and Leading/Trailing flags are distinct features which we will demo together.
            // (It is possible to submit regular tabs with Leading/Trailing flags, or TabItemButton tabs without Leading/Trailing flags...
            // but they tend to make more sense together)
            static bool show_leading_button = true;
            static bool show_trailing_button = true;
            ImGui::Checkbox("Show Leading TabItemButton()", &show_leading_button);
            ImGui::Checkbox("Show Trailing TabItemButton()", &show_trailing_button);

            // Expose some other flags which are useful to showcase how they interact with Leading/Trailing tabs
            static ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown;
            ImGui::CheckboxFlags("ImGuiTabBarFlags_TabListPopupButton", &tab_bar_flags, ImGuiTabBarFlags_TabListPopupButton);
            if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyResizeDown", &tab_bar_flags, ImGuiTabBarFlags_FittingPolicyResizeDown))
                tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyResizeDown);
            if (ImGui::CheckboxFlags("ImGuiTabBarFlags_FittingPolicyScroll", &tab_bar_flags, ImGuiTabBarFlags_FittingPolicyScroll))
                tab_bar_flags &= ~(ImGuiTabBarFlags_FittingPolicyMask_ ^ ImGuiTabBarFlags_FittingPolicyScroll);

            if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
            {
                // Demo a Leading TabItemButton(): click the "?" button to open a menu
                if (show_leading_button)
                    if (ImGui::TabItemButton("?", ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip))
                        ImGui::OpenPopup("MyHelpMenu");
                if (ImGui::BeginPopup("MyHelpMenu"))
                {
                    ImGui::Selectable("Hello!");
                    ImGui::EndPopup();
                }

                // Demo Trailing Tabs: click the "+" button to add a new tab.
                // (In your app you may want to use a font icon instead of the "+")
                // We submit it before the regular tabs, but thanks to the ImGuiTabItemFlags_Trailing flag it will always appear at the end.
                if (show_trailing_button)
                    if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
                        active_tabs.push_back(next_tab_id++); // Add new tab

                // Submit our regular tabs
                for (int n = 0; n < active_tabs.size(); )
                {
                    bool open = true;
                    char name[16];
                    snprintf(name, IM_ARRAYSIZE(name), "%04d", active_tabs[n]);
                    if (ImGui::BeginTabItem(name, &open, ImGuiTabItemFlags_None))
                    {
                        ImGui::Text("This is the %s tab!", name);
                        ImGui::EndTabItem();
                    }

                    if (!open)
                        active_tabs.erase(active_tabs.begin() + n);
                    else
                        n++;
                }

                ImGui::EndTabBar();
            }
        ImGui::End();
    }

private:
    void list_directory_content() {
        std::vector<std::filesystem::path> directories;
        std::vector<std::filesystem::path> bin_files;
        std::vector<std::filesystem::path> files;

        IsBinary("",true);

        for (auto& entry : fs::directory_iterator(current_directory)) {
            const auto& path = entry.path();
            bool isDir = fs::is_directory(path);
            bool isBinary = IsBinary(path);

            if (isDir)
                directories.push_back(path);
            else if (isBinary)
                bin_files.push_back(path);
            else
                files.push_back(path);
        }

        ImGui::Separator();
        
        if (!directories.empty()){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.71f, 0.25f, 1.0f));
            for (const auto& dir : directories) {
                if (ImGui::Selectable((dir.filename().string() + "/").c_str())) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        backwards_directory_history.push(current_directory);
                        current_directory = dir;
                    }
                }
            }
            ImGui::PopStyleColor();
            ImGui::Separator();
        }

        for (const auto& file : files) {
            if (ImGui::Selectable(file.filename().string().c_str())) {
                std::cout << "File to open : " << file.filename().string() << std::endl; 
            }
        }

        for (const auto& file : bin_files)
            ImGui::Selectable(file.filename().string().c_str(), false, ImGuiSelectableFlags_Disabled);

    }

    bool IsBinary(const fs::path& path,bool reset=false) {
        static bool b = false;
        if (reset)
            b = reset;
        else
            b = !b;
        return b;
    }

private:
    void GoBack() {
        if (!backwards_directory_history.empty()) {
            forwards_directory_history.push(current_directory);
            current_directory = backwards_directory_history.top();
            backwards_directory_history.pop();
            std::strncpy(path_buffer, current_directory.string().c_str(), sizeof(path_buffer));
        }
    }

    void GoUp() {
        fs::path parent = current_directory.parent_path();
        if (parent != current_directory) {
            backwards_directory_history.push(current_directory);
            current_directory = parent;
            std::strncpy(path_buffer, current_directory.string().c_str(), sizeof(path_buffer));
            while(!forwards_directory_history.empty())
                forwards_directory_history.pop();
        }
    }

    void GoForward() {
        if (!forwards_directory_history.empty()) {
            backwards_directory_history.push(current_directory);
            current_directory = forwards_directory_history.top();
            forwards_directory_history.pop();
            std::strncpy(path_buffer, current_directory.string().c_str(), sizeof(path_buffer));
        }
    }

    void SetCurrentDirectory(const char* new_path) {
        fs::path new_dir(new_path);

        if (fs::exists(new_dir) && fs::is_directory(new_dir)) {
            backwards_directory_history.push(current_directory);
            current_directory = new_dir;
            while(!forwards_directory_history.empty())
                forwards_directory_history.pop();
        }

        std::strncpy(path_buffer, current_directory.string().c_str(), sizeof(path_buffer));
    }

    void BrowseDialog() {
        const char* selected_path = tinyfd_selectFolderDialog(
            "Select a folder",
            current_directory.string().c_str()
        );

        if (selected_path != NULL){
            backwards_directory_history.push(current_directory);
            current_directory = selected_path;
            std::strncpy(path_buffer, selected_path, sizeof(path_buffer));
            while(!forwards_directory_history.empty())
                forwards_directory_history.pop();
        }
    }

private:
    char path_buffer[512];
    fs::path current_directory = "C:\\Users\\crevelim\\Downloads";
    std::stack<fs::path> backwards_directory_history;
    std::stack<fs::path> forwards_directory_history;
};

int main() {
    ReBit app;
    if (app.Construct())
        app.Start();
    return 0;
}
