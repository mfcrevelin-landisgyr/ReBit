#include "imgui_wrapper.h"
#include <filesystem>
#include <map>
#include <vector>
#include <string>

namespace fs = std::filesystem;

class CSVEditor : public ImGui::Wrapper
{
public:
    CSVEditor() {
        title = "CSV Editor";
    }

    bool Create() override {
        // Initialization for the editor
        return true;
    }

    bool Update() override {
        ShowProjectPanel();
        ShowFilePanels();
        return true;
    }

private:
    // The current directory path
    fs::path currentDir;

    // Map of opened files with their data
    std::map<std::string, std::map<std::string, std::vector<std::string>>> openFiles;

    // Show the project panel on the left side
    void ShowProjectPanel() {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        ImGui::Begin("Project Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        if (ImGui::Button("Select Directory")) {
            // Open a file dialog to select a directory
            const char* path = tinyfd_selectFolderDialog("Select a Directory", currentDir.string().c_str());
            if (path) {
                currentDir = fs::path(path);
            }
        }

        // List files and directories
        if (!currentDir.empty()) {
            ShowDirectoryContents(currentDir);
        }

        ImGui::End();
    }

    // Show contents of a directory in the project panel
    void ShowDirectoryContents(const fs::path& dir) {
        for (auto& entry : fs::directory_iterator(dir)) {
            const auto& path = entry.path();
            bool isDir = fs::is_directory(path);
            bool isBinary = IsBinary(path);

            if (isDir) {
                if (ImGui::Selectable((path.filename().string() + "/").c_str())) {
                    currentDir = path;  // Navigate into the directory
                }
            }
            else if (!isBinary) {
                if (ImGui::Selectable(path.filename().string().c_str())) {
                    OpenFile(path);  // Open the file when clicked
                }
            }
            else {
                ImGui::Selectable(path.filename().string().c_str(), false);  // Disabled binary files
            }
        }

        // Parent directory navigation
        if (currentDir != fs::path()) {
            if (ImGui::Button("Up")) {
                currentDir = currentDir.parent_path();  // Navigate up one directory
            }
        }
    }

    // Determine if a file is binary based on its content
    bool IsBinary(const fs::path& path) {
        try {
            std::ifstream file(path, std::ios::binary);
            char ch;
            while (file.get(ch)) {
                if (ch == '\0') {  // Null character found, likely binary
                    return true;
                }
            }
        }
        catch (...) {
            return false;  // In case of any error, treat as non-binary
        }
        return false;  // Assume non-binary if no null character
    }

    // Open a file and load its data
    void OpenFile(const fs::path& filePath) {
        // Load the CSV data (using a placeholder method for now)
        auto table = LoadFile(filePath.string());
        openFiles[filePath.string()] = table;

        // Show the file panel
        ShowFilePanel(filePath.string(), table);
    }

    // Show the file panel with editable table
    void ShowFilePanel(const std::string& filePath, std::map<std::string, std::vector<std::string>>& table) {
        ImGui::Begin(filePath.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // Display the table in a grid format
        for (auto& [key, values] : table) {
            ImGui::Text("%s", key.c_str());  // Column header
            for (size_t i = 0; i < values.size(); ++i) {
                ImGui::InputText(("Cell " + std::to_string(i)).c_str(), &values[i]);
            }
        }

        ImGui::End();
    }

    // Dummy method to load a file, returns a simple table for now
    std::map<std::string, std::vector<std::string>> LoadFile(const std::string& filePath) {
        // Simulating loading a file into a table format
        std::map<std::string, std::vector<std::string>> table;
        table["Header 1"] = {"Row 1", "Row 2", "Row 3"};
        table["Header 2"] = {"Value 1", "Value 2", "Value 3"};
        return table;
    }
};

int main() {
    CSVEditor app;
    if (app.Construct()) {
        app.Start();
    }
    return 0;
}
