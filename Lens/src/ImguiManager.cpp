#include "LensPch.h"
#include "ImguiManager.h"


namespace lens
{
    ImguiManager::ImguiManager()
    {
    }

    ImguiManager::~ImguiManager()
    {
        Shutdown();
    }

    bool ImguiManager::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* device_context)
    {

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        ImGui::StyleColorsDark();

        if (!ImGui_ImplWin32_Init(hwnd))
            return false;
        if (!ImGui_ImplDX11_Init(device, device_context))
            return false;

        return true;
    }

    void ImguiManager::ShowWindow()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void ImguiManager::Shutdown()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void ImguiManager::ShowDemoWindow()
    {
        ImGui::ShowDemoWindow();
    }
}
