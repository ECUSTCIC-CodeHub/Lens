#include "LensPch.h"
#include "gui/UIManager.h"
#include "gui/DemoPanel.h"
#include "gui/DebugPanel.h"
#include "gui/CapturePanel.h"
#include "capturer/WGCCapturer.h"
#include "Log.h"

namespace lens
{
    void UIManager::InitializeAllPanels(capturer::WGCCapturer* capturer)
    {
        LOG_INFO("UIManager: Initializing all panels...");

        // 注册核心面板（Demo、Debug 等）
        RegisterCorePanels();

        // 注册捕获相关面板（Capture、Recorder 等）
        RegisterCapturePanels(capturer);

        LOG_INFO("UIManager: All panels initialized. Total panels: {}", m_panels.size());
    }

    void UIManager::RegisterCorePanels()
    {
        LOG_INFO("UIManager: Registering core panels...");

        // 注册 DemoPanel
        //auto* demoPanel = AddPanel<DemoPanel>();
        //if (demoPanel)
        //{
        //    LOG_INFO("  - DemoPanel registered");
        //    demoPanel->SetVisible(false); // 默认隐藏 Demo 面板
        //}
        //else
        //{
        //    LOG_ERROR("  - Failed to register DemoPanel");
        //}

        // 注册 DebugPanel
        auto* debugPanel = AddPanel<DebugPanel>();
        if (debugPanel)
        {
            LOG_INFO("  - DebugPanel registered");
            debugPanel->SetVisible(true); // 默认显示 Debug 面板
        }
        else
        {
            LOG_ERROR("  - Failed to register DebugPanel");
        }
    }

    void UIManager::RegisterCapturePanels(capturer::WGCCapturer* capturer)
    {
        LOG_INFO("UIManager: Registering capture panels...");

        // 注册 CapturePanel（需要 capturer 依赖）
        if (capturer)
        {
            auto* capturePanel = AddPanel<CapturePanel>();
            if (capturePanel)
            {
                LOG_INFO("  - CapturePanel registered");
                capturePanel->SetCapturer(capturer);
                capturePanel->SetVisible(true); // 默认显示 Capture 面板
            }
            else
            {
                LOG_ERROR("  - Failed to register CapturePanel");
            }
        }
        else
        {
            LOG_WARN("  - Capturer is null, skipping CapturePanel registration");
        }
    }

    UIPanel* UIManager::GetPanel(const std::string& name)
    {
        auto it = m_panelMap.find(name);
        return (it != m_panelMap.end()) ? it->second : nullptr;
    }

    void UIManager::ShowPanel(const std::string& name, bool show)
    {
        UIPanel* panel = GetPanel(name);
        if (panel)
        {
            panel->SetVisible(show);
        }
    }

    void UIManager::RemovePanel(const std::string& name)
    {
        auto it = m_panelMap.find(name);
        if (it == m_panelMap.end())
        {
            return;
        }

        UIPanel* panel = it->second;
        panel->Shutdown();

        m_panels.erase(
            std::remove_if(m_panels.begin(), m_panels.end(),
                [panel](const std::unique_ptr<UIPanel>& ptr) {
                    return ptr.get() == panel;
                }),
            m_panels.end()
        );

        m_panelMap.erase(it);
    }

    void UIManager::RenderAll()
    {
        if (m_showMenu)
        {
            RenderMenu();
        }

        for (auto& panel : m_panels)
        {
            if (panel->IsVisible())
            {
                panel->Render();
            }
        }
    }

    void UIManager::RenderMenu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Panels"))
            {
                for (auto& [name, panel] : m_panelMap)
                {
                    bool visible = panel->IsVisible();
                    if (ImGui::MenuItem(name.c_str(), nullptr, &visible))
                    {
                        panel->SetVisible(visible);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void UIManager::Shutdown()
    {
        LOG_INFO("UIManager: Shutting down all panels...");
        for (auto& panel : m_panels)
        {
            panel->Shutdown();
        }
        m_panels.clear();
        m_panelMap.clear();
        LOG_INFO("UIManager: All panels shut down");
    }
}