#pragma once
#include <Windows.h>
#include "RHI_dx11.h"
#include "ImguiManager.h"

namespace lens
{
    class Application
    {
    public:
        Application(HINSTANCE hInstance, int mCmdShow,
            const wchar_t* className = L"LensWindowClass", const wchar_t* title = L"Lens Application");

        ~Application();

        void Initialize();

        int Run();

        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        bool CreateWindow_(int width = 800, int height = 600);

        HINSTANCE m_hInstance;
        int m_nCmdShow;
        HWND m_hwnd;

        RHI_dx11* m_rhi;
        ImguiManager* m_imgui;

        std::wstring m_className;
        std::wstring m_title;
        bool isExit;
    };
}