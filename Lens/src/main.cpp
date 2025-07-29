#include "LensPch.h"
#pragma comment(lib, "windowsapp")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    init_apartment(winrt::apartment_type::single_threaded);

    WNDCLASSEX wcex;
    {
        wcex.cbSize = sizeof(WNDCLASSEX);
        //wcex.style = CS_DROPSHADOW | CS_HREDRAW | CS_VREDRAW;   // 窗口样式
        wcex.style = CS_HREDRAW | CS_VREDRAW;                   // 窗口样式
        wcex.lpfnWndProc = DefWindowProc;                       // 消息回调时的回调函数
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance; 
        wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"LensWindowClass";
        wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        RegisterClassEx(&wcex);
    }

    RegisterClassEx(&wcex);

    HWND hwnd = CreateWindowEx(
        0,                              // 扩展样式
        L"LensWindowClass",             // 窗口类名
        L"Lens Application",            // 窗口标题
        WS_OVERLAPPEDWINDOW,            // 窗口样式
        CW_USEDEFAULT, CW_USEDEFAULT,   // 初始位置
        800, 600,                       // 初始大小
        nullptr,                        // 父窗口句柄
        nullptr,                        // 菜单句柄
        hInstance,                      // 应用程序实例句柄
        nullptr                         // 附加参数
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
