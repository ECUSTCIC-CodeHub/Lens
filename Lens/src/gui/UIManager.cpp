#include "LensPch.h"
#include "gui/UIManager.h"

namespace lens
{
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
        for (auto& panel : m_panels)
        {
            panel->Shutdown();
        }
        m_panels.clear();
        m_panelMap.clear();
    }
}