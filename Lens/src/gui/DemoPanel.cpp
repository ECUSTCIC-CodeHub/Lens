#include "LensPch.h"
#include "gui/DemoPanel.h"

namespace lens
{
    DemoPanel::DemoPanel()
    {
    }

    void DemoPanel::Initialize()
    {
        LOG_INFO("DemoPanel initialized");
    }

    void DemoPanel::Shutdown()
    {
        LOG_INFO("DemoPanel shutdown");
    }

    void DemoPanel::Render()
    {
        if (!m_visible)
            return;

        ImGui::Begin("Demo Panel", &m_visible);

        ImGui::Text("Hello from the new UI System!");
        ImGui::Separator();

        if (ImGui::Button("Click me"))
        {
            m_counter++;
            LOG_INFO("Demo button clicked {} times", m_counter);
        }

        ImGui::Text("Button clicks: %d", m_counter);

        ImGui::Separator();
        ImGui::ColorEdit4("Color", m_color);

        ImGui::Text("Application Performance:");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

        ImGui::End();
    }
}