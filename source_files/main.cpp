#include "imgui_wrapper.h"
#include <string>

class ReBit : public ImGui::Wrapper
{
public:
    ReBit() {
        title = "ReBit";
    }

public:
    bool Create() override
    {
        // User setup here. Dummy for now.
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("IBMPlexMono-Regular.ttf", 18.0f);
        return true;
    }

    bool Update() override
    {
        // User code here. Dummy for now.
        ImGui::ShowDemoWindow();

        return true;
    }
};

int main()
{
    ReBit app;
    if (app.Construct())
        app.Start();
    return 0;
}