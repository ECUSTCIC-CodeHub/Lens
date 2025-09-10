#include "LensPch.h"

#pragma comment(lib, "windowsapp")

#include "Application.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    lens::Application app(hInstance, nCmdShow);

    // 初始化组件
    app.Initialize();

    // 执行流总入口
    auto res = app.Run();

    return res;
}
