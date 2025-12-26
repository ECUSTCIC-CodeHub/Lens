#include "LensPch.h"
#include "WGCTestApp.h"

#pragma comment(lib, "windowsapp")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    LOG_INFO("Starting WGC Test Application");
    lens::WGCTestApp app(hInstance, nCmdShow);

    if (!app.Initialize())
    {
        LOG_ERROR("Failed to initialize application");
        return 1;
    }

    return app.Run();
}
