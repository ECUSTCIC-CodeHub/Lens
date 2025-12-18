#include "LensPch.h"
#include "Application.h"
#include "graphics/GraphicsDevice.h"
#include "ImguiManager.h"
#include "gui/DemoPanel.h"
#include "gui/DebugPanel.h"
#include "Log.h"


namespace lens
{
    Application::Application(HINSTANCE hInstance, int mCmdShow,
        const wchar_t* className, const wchar_t* title)
        : m_hInstance(hInstance), 
          m_nCmdShow(mCmdShow), 
          m_className(className), 
          m_title(title),
          isExit(false)
    {
        init_apartment(winrt::apartment_type::single_threaded);
    }

    Application::~Application()
    {
        if (m_imgui) {
            delete m_imgui;
            m_imgui = nullptr;
        }
        if (m_graphicsDevice) {
            delete m_graphicsDevice;
            m_graphicsDevice = nullptr;
        }
    }

    void Application::Initialize()
    {
        if (!CreateLenWindow(800, 600))
        {
            LOG_ERROR("Failed to create window");
            return;
        }

        m_graphicsDevice = new graphics::GraphicsDevice();
        graphics::GraphicsDevice::Desc deviceDesc{};
        deviceDesc.windowHandle = m_hwnd;
        deviceDesc.width = 800;
        deviceDesc.height = 600;
        deviceDesc.enableDebug = true;

        if (!m_graphicsDevice->Initialize(deviceDesc)) {
            LOG_ERROR("Failed to initialize graphics device");
            return;
        }

        m_imgui = new ImguiManager();
        m_imgui->Initialize(m_hwnd, m_graphicsDevice->GetDevice(), m_graphicsDevice->GetContext());

        // Enable menu
        m_imgui->SetMenuVisible(true);

        // Register UI panels
        UIManager* uiManager = m_imgui->GetUIManager();
        uiManager->AddPanel<DemoPanel>();
        uiManager->AddPanel<DebugPanel>();
        uiManager->ShowPanel("Debug", true);

        LOG_INFO("Application initialized successfully ");
    }

    int Application::Run()
    {
        MSG msg = { 0 };
        while (!isExit)
        {
            // Handle Windows messages
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    isExit = true;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                m_imgui->HandleMessage(msg);
            }

            if (isExit)
                break;

            // Begin frame
            m_graphicsDevice->BeginFrame();
            m_imgui->BeginFrame();

            // Render all UI panels
            m_imgui->GetUIManager()->RenderAll();

            // End ImGui frame and render
            m_imgui->EndFrame();

            // End frame and present
            m_graphicsDevice->EndFrame();
            m_graphicsDevice->Present(true);
        }

        return static_cast<int>(msg.wParam);
    }

    LRESULT CALLBACK Application::WindowProcProxy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        Application* app = nullptr;
        if(uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            app = reinterpret_cast<Application*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        }
        else
        {
            app = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }
        return app->WndProc(hwnd, uMsg, wParam, lParam);
    }

    LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            {
                if (this->m_graphicsDevice != nullptr && wParam != SIZE_MINIMIZED)
                {
                    int width = LOWORD(lParam);
                    int height = HIWORD(lParam);

                    // 首先调整图形设备的交换链大小
                    this->m_graphicsDevice->Resize(width, height);

                    // 更新ImGui的显示尺寸
                    ImGuiIO& io = ImGui::GetIO();
                    io.DisplaySize = ImVec2((float)width, (float)height);

                    float dpiScale = GetDpiForWindow(hWnd) / 96.0f; // 96 是默认 DPI
                    io.DisplayFramebufferScale = ImVec2(dpiScale, dpiScale);
                }
            }
            return 0;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    bool Application::CreateLenWindow(int width, int height)
    {
        // 注册窗口类
        WNDCLASSEX wcex = { 0 };
        {
            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = WindowProcProxy;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = m_hInstance;
            wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wcex.lpszMenuName = nullptr;
            wcex.lpszClassName = m_className.c_str();
            wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        }

        if (!RegisterClassEx(&wcex))
        {
            return false;
        }

        // 创建窗口
        m_hwnd = CreateWindowEx(
            0,
            m_className.c_str(),
            m_title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            width, height,
            nullptr,
            nullptr,
            m_hInstance,
            this // WindowProcProxys()使用
        );

        if (!m_hwnd)
        {
            return false;
        }

        ShowWindow(m_hwnd, m_nCmdShow);
        UpdateWindow(m_hwnd);

        LOG_INFO("Window created successfule");
        return true;
    }
}
