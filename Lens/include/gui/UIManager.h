#pragma once

#include "UIPanel.h"
#include <vector>
#include <memory>
#include <map>
#include <type_traits>

namespace lens
{
    class UIManager
    {
    private:
        std::vector<std::unique_ptr<UIPanel>> m_panels;
        std::map<std::string, UIPanel*> m_panelMap;
        bool m_showMenu = true;

    public:
        UIManager() = default;
        ~UIManager() = default;

        template<typename T, typename... Args>
        T* AddPanel(Args&&... args)
        {
            static_assert(std::is_base_of<UIPanel, T>::value, "T must inherit from UIPanel");

            auto panel = std::make_unique<T>(std::forward<Args>(args)...);
            T* panelPtr = panel.get();

            const char* name = panelPtr->GetName();
            if (m_panelMap.find(name) != m_panelMap.end())
            {
                return nullptr; // Panel with this name already exists
            }

            m_panelMap[name] = panelPtr;
            panelPtr->Initialize();
            m_panels.push_back(std::move(panel));

            return panelPtr;
        }

        UIPanel* GetPanel(const std::string& name);
        void ShowPanel(const std::string& name, bool show = true);
        void RemovePanel(const std::string& name);

        void RenderAll();
        void RenderMenu();

        void SetMenuVisible(bool visible) { m_showMenu = visible; }
        bool IsMenuVisible() const { return m_showMenu; }

        void Shutdown();
    };
}