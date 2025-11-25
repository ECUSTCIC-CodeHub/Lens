#pragma once

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include "gui/UIManager.h"
#include <memory>

namespace lens
{
    class ImguiManager
    {
    public:
        ImguiManager();
        ~ImguiManager();

        bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

        void HandleMessage(MSG& msg);
        void BeginFrame();
        void EndFrame();
        void Shutdown();

        UIManager* GetUIManager() { return m_uiManager.get(); }

        void SetMenuVisible(bool visible) { m_menuVisible = visible; m_uiManager->SetMenuVisible(visible); }
        bool IsMenuVisible() const { return m_menuVisible; }

    private:
        std::unique_ptr<UIManager> m_uiManager;
        bool m_initialized = false;
        bool m_menuVisible = true;
    };
}