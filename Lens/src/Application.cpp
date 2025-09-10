#include "LensPch.h"
#include "Application.h"
#include "RHI_dx11.h"
#include "ImguiManager.h"
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

    }

    void Application::Initialize()
    {
        if (!CreateWindow_(800, 600))
        {
            LOG_ERROR("Failed to create window");
            return;
        }
        
        m_rhi = RHI_dx11::Create(m_hwnd, 800, 600);
        m_rhi->Initialize(m_hwnd, 800, 600);
        m_imgui = new ImguiManager();
        m_imgui->Initialize(m_hwnd, m_rhi->GetDevice(), m_rhi->GetContext());
        int a = 1;
        LOG_INFO("Application initialized successfully {}",  1);
    }

    int Application::Run()
    {
        MSG msg;
        while (!isExit)
        {
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

            m_imgui->ShowWindow();
            
            const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
            m_rhi->GetContext()->OMSetRenderTargets(1, &m_rhi->defaultRTV, nullptr);
            m_rhi->GetContext()->ClearRenderTargetView(m_rhi->defaultRTV, clear_color_with_alpha);

            m_imgui->Draw();

            m_rhi->GetSwapChain()->Present(1, 0);

        }
        return static_cast<int>(msg.wParam);
    }

    LRESULT CALLBACK Application::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    bool Application::CreateWindow_(int width, int height)
    {
        // 注册窗口类
        WNDCLASSEX wcex = { 0 };
        {
            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = WindowProc;
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
            nullptr
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
