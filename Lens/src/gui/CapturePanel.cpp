#include "LensPch.h"
#include "gui/CapturePanel.h"
#include "Application.h"
#include "Log.h"

namespace lens
{
    CapturePanel::CapturePanel()
        : m_capturer(nullptr)
    {
    }

    void CapturePanel::Initialize()
    {
        LOG_INFO("CapturePanel initialized");
    }

    void CapturePanel::Shutdown()
    {
        LOG_INFO("CapturePanel shutdown");
    }

    void CapturePanel::Render()
    {
        if (!m_visible)
            return;

        // Set default window size (only on first use)
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

        // Update cached frame if new one is available
        if (m_capturer && m_capturer->HasNewFrame())
        {
            auto frame = m_capturer->GetLatestFrame();
            if (frame)
            {
                m_lastFrame = frame;
            }
        }

        if (ImGui::Begin("Capture", &m_visible))
        {
            // Render cached frame
            if (m_lastFrame && m_lastFrame->GetSRV())
            {
                // Get texture dimensions
                uint32_t texWidth = m_lastFrame->GetWidth();
                uint32_t texHeight = m_lastFrame->GetHeight();

                // Convert SRV to ImTextureID (ID3D11ShaderResourceView*)
                ImTextureID textureId = reinterpret_cast<ImTextureID>(m_lastFrame->GetSRV());

                // Get available content region
                ImVec2 availSize = ImGui::GetContentRegionAvail();

                // Calculate display size maintaining aspect ratio
                float texAspect = static_cast<float>(texWidth) / static_cast<float>(texHeight);
                float availAspect = availSize.x / availSize.y;

                float displayWidth, displayHeight;

                // Fit to available space while maintaining aspect ratio
                if (availAspect > texAspect)
                {
                    // Available space is wider than texture
                    displayHeight = availSize.y;
                    displayWidth = displayHeight * texAspect;
                }
                else
                {
                    // Available space is taller than texture
                    displayWidth = availSize.x;
                    displayHeight = displayWidth / texAspect;
                }

                // Center the image
                ImGui::SetCursorPos(
                    ImVec2(
                        (availSize.x - displayWidth) * 0.5f,
                        (availSize.y - displayHeight) * 0.5f
                    )
                );

                // Render the image
                ImGui::Image(textureId, ImVec2(displayWidth, displayHeight));
            }
            else
            {
                // Show placeholder text when no frame is available
                ImVec2 availSize = ImGui::GetContentRegionAvail();
                ImGui::SetCursorPos(ImVec2(availSize.x * 0.5f, availSize.y * 0.5f));

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::Text("No capture");
                ImGui::PopStyleColor();
            }
        }
        ImGui::End();
    }
}
