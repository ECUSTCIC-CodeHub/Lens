#pragma once
#include <Windows.h>
#include "graphics/GraphicsDevice.h"
#include "graphics/Shader.h"
#include "ImguiManager.h"
#include "capturer/WGCCapturer.h"
#include <memory>
#include <d3dcompiler.h>
#include <wrl/client.h>

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

        static LRESULT CALLBACK WindowProcProxy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // Public getter for capturer access from UI panels
        capturer::WGCCapturer* GetCapturer() const { return m_capturer.get(); }

    private:
        bool CreateLenWindow(int width = 800, int height = 600);
        bool CreateCaptureShaders();
        bool CreateCaptureSampler();
        bool CompileShader(const std::string& code, const char* entryPoint, const char* target, Microsoft::WRL::ComPtr<ID3DBlob>& blob);

        LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        HINSTANCE m_hInstance;
        int m_nCmdShow;
        HWND m_hwnd;

        graphics::GraphicsDevice* m_graphicsDevice;
        ImguiManager* m_imgui;

        // WGC Capture related
        std::unique_ptr<capturer::WGCCapturer> m_capturer;
        graphics::Shader m_captureShader;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_captureSampler;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_captureConstantBuffer;
        std::shared_ptr<graphics::Texture> m_lastCapturedFrame;

        int width;
        int height;
        std::wstring m_className;
        std::wstring m_title;
        bool isExit;
    };
}