#include "LensPch.h"
#include "gui/DebugPanel.h"

namespace lens
{
    void DebugPanel::Initialize()
    {
        LOG_INFO("DebugPanel initialized");
    }

    void DebugPanel::Shutdown()
    {
        LOG_INFO("DebugPanel shutdown");
    }

    void DebugPanel::Render()
    {
        if (!m_visible)
            return;

        if (ImGui::Begin("Debug Info", &m_visible))
        {
            ImGui::Text("System Information");
            ImGui::Separator();

            ImGuiIO& io = ImGui::GetIO();
            ImGui::Text("Dear ImGui Version: %s", IMGUI_VERSION);
            ImGui::Text("Display Size: %.0f x %.0f", io.DisplaySize.x, io.DisplaySize.y);
            ImGui::Text("Mouse Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);

            ImGui::Separator();

            ImGui::Text("Performance");
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
            ImGui::Text("Vertices: %d", io.MetricsRenderVertices);
            ImGui::Text("Indices: %d", io.MetricsRenderIndices);

            ImGui::Separator();

            static bool showDemoWindow = false;
            if (ImGui::Button(showDemoWindow ? "Hide ImGui Demo" : "Show ImGui Demo"))
            {
                showDemoWindow = !showDemoWindow;
            }

            if (showDemoWindow)
            {
                ImGui::ShowDemoWindow(&showDemoWindow);
            }
        }
        ImGui::End();
    }
}