#include "LensPch.h"

#pragma comment(lib, "windowsapp")

#include "Application.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    lens::Application window(hInstance, nCmdShow);

    window.Create(800, 600);

    auto res = window.Run();

    return res;
}
