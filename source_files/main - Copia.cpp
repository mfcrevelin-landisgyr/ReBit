#include "imgui_wrapper.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <map>

namespace fs = std::filesystem;

class ReBit : public ImGui::Wrapper
{
public:
    ReBit() {
        title = "Rebit";
    }

    bool Create() override {
        return true;
    }

    bool Update() override {
        ShowProjectPanel();
        ShowFilePanels();
        return true;
    }

private:
    fs::path currentDir;
    std::map<std::string, std::map<std::string, std::vector<std::string>>> openFiles;

    void ShowProjectPanel() {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        ImGui::Begin("Project Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (ImGui::Button("Select Directory")) {
            const char* path = tinyfd_selectFolderDialog("Select a Directory", currentDir.string().c_str());
            if (path) {
                currentDir = fs::path(path);
            }
        }

        if (!currentDir.empty()) {
            ShowDirectoryContents(currentDir);
        }

        ImGui::End();
    }

    void ShowDirectoryContents(const fs::path& dir) {
        for (auto& entry : fs::directory_iterator(dir)) {
            const auto& path = entry.path();
            bool isDir = fs::is_directory(path);
            bool isBinary = IsBinary(path);

            if (isDir) {
                if (ImGui::Selectable((path.filename().string() + "/").c_str())) {
                    currentDir = path;
                }
            }
            else if (!isBinary) {
                if (ImGui::Selectable(path.filename().string().c_str())) {
                    OpenFile(path);
                }
            }
            else {
                ImGui::Selectable(path.filename().string().c_str(), false);
            }
        }

        if (currentDir != fs::path()) {
            if (ImGui::Button("Up")) {
                currentDir = currentDir.parent_path();
            }
        }
    }

    bool IsBinary(const fs::path& path) {
        try {
            std::ifstream file(path, std::ios::binary);
            char ch;
            while (file.get(ch)) {
                if (ch == '\0')
                    return true;
            }
        } catch (...) {
            return true;
        }
        return false;
    }

    void OpenFile(const fs::path& filePath) {
        auto table = LoadFile(filePath.string());
        openFiles[filePath.string()] = table;
        ShowFilePanel(filePath.string(), table);
    }

    void ShowFilePanel(const std::string& filePath, std::map<std::string, std::vector<std::string>>& table) {
        ImGui::Begin(filePath.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        for (auto& [key, values] : table) {
            ImGui::Text("%s", key.c_str());
            for (size_t i = 0; i < values.size(); ++i)
                ImGui::InputText(("Cell " + std::to_string(i)).c_str(), &values[i]);
        }

        ImGui::End();
    }

    std::map<std::string, std::vector<std::string>> LoadFile(const std::string& filePath) {
        std::map<std::string, std::vector<std::string>> table;
        table["Header 1"] = {"Row 1", "Row 2", "Row 3"};
        table["Header 2"] = {"Value 1", "Value 2", "Value 3"};
        return table;
    }
};

int main() {
    ReBit app;
    if (app.Construct())
        app.Start();
    return 0;
}
