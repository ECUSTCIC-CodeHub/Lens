#include "LensPch.h"
#include "ImguiManager.h"
#include "Log.h"

namespace lens
{
    ImguiManager::ImguiManager()
        : m_uiManager(std::make_unique<UIManager>())
    {
    }

    ImguiManager::~ImguiManager()
    {
        Shutdown();
    }

    bool ImguiManager::Initialize(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        if (m_initialized)
        {
            LOG_WARN("ImguiManager already initialized");
            return true;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        // 启用键盘导航功能
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // 启用窗口停靠功能
#ifdef IMGUI_HAS_DOCKING
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

        ImGui::StyleColorsDark();

        if (!ImGui_ImplWin32_Init(hwnd))
        {
            LOG_ERROR("Failed to initialize ImGui Win32 backend");
            return false;
        }

        if (!ImGui_ImplDX11_Init(device, context))
        {
            LOG_ERROR("Failed to initialize ImGui DirectX 11 backend");
            return false;
        }

        m_initialized = true;
        LOG_INFO("ImguiManager initialized successfully");
        return true;
    }

    void ImguiManager::HandleMessage(MSG& msg)
    {
        if (!m_initialized)
            return;

        extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        ImGui_ImplWin32_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam);
    }

    void ImguiManager::BeginFrame()
    {
        if (!m_initialized)
            return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void ImguiManager::EndFrame()
    {
        if (!m_initialized)
            return;

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void ImguiManager::Shutdown()
    {
        if (!m_initialized)
            return;

        if (m_uiManager)
        {
            m_uiManager->Shutdown();
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        m_initialized = false;
        LOG_INFO("ImguiManager shutdown complete");
    }
}