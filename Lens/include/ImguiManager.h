#pragma once

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

namespace lens
{
    class ImguiManager
    {
    public:
        ImguiManager();
        ~ImguiManager();

        bool Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context);
        
        void ShowWindow();

        void Shutdown();

        void ShowDemoWindow();

    private:
    };
}