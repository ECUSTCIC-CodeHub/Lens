#pragma once
#include <Windows.h>
#include "graphics/GraphicsDevice.h"
#include "graphics/Shader.h"
#include "capturer/WGCCapturer.h"
#include <memory>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace lens
{
    class WGCTestApp
    {
    public:
        WGCTestApp(HINSTANCE hInstance, int nCmdShow);
        ~WGCTestApp();

        bool Initialize();
        int Run();

    private:
        static LRESULT CALLBACK WindowProcProxy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        bool CreateRenderWindow(int width, int height);
        bool CreateShaders();
        bool CompileShader(const std::string& code, const char* entryPoint, const char* target, Microsoft::WRL::ComPtr<ID3DBlob>& blob);
        bool CreateSampler();
        bool SelectAndStartCapture();
        void RenderFrame();
        void RenderTexture(std::shared_ptr<graphics::Texture> texture);

        HINSTANCE m_hInstance;
        int m_nCmdShow;
        HWND m_hwnd;

        graphics::GraphicsDevice* m_graphicsDevice;
        graphics::Shader m_shader;
        std::unique_ptr<capturer::WGCCapturer> m_capturer;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

        uint32_t m_width;
        uint32_t m_height;
        std::wstring m_className;
        std::wstring m_title;
        bool m_isInitialized;
        bool m_shouldCapture;
    };
}
