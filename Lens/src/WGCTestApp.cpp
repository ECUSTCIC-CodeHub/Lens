#include "LensPch.h"
#include "WGCTestApp.h"
#include "graphics/Texture.h"
#include "Log.h"
#include <d3d11.h>
#include <iostream>

#pragma comment(lib, "windowsapp")

namespace lens
{
    static WGCTestApp* g_app = nullptr;

    WGCTestApp::WGCTestApp(HINSTANCE hInstance, int nCmdShow)
        : m_hInstance(hInstance)
        , m_nCmdShow(nCmdShow)
        , m_hwnd(nullptr)
        , m_graphicsDevice(nullptr)
        , m_width(1280)
        , m_height(720)
        , m_className(L"WGCTestWindowClass")
        , m_title(L"WGC Capture Test")
        , m_isInitialized(false)
        , m_shouldCapture(true)
    {
        g_app = this;
        init_apartment(winrt::apartment_type::single_threaded);
    }

    WGCTestApp::~WGCTestApp()
    {
        m_capturer.reset();

        if (m_graphicsDevice)
        {
            delete m_graphicsDevice;
            m_graphicsDevice = nullptr;
        }

        if (g_app == this)
            g_app = nullptr;
    }

    bool WGCTestApp::Initialize()
    {
        if (!CreateRenderWindow(static_cast<int>(m_width), static_cast<int>(m_height)))
        {
            LOG_ERROR("Failed to create render window");
            return false;
        }

        m_graphicsDevice = new graphics::GraphicsDevice();
        graphics::GraphicsDevice::Desc deviceDesc{};
        deviceDesc.windowHandle = m_hwnd;
        deviceDesc.width = m_width;
        deviceDesc.height = m_height;
        deviceDesc.enableDebug = true;

        if (!m_graphicsDevice->Initialize(deviceDesc))
        {
            LOG_ERROR("Failed to initialize graphics device");
            return false;
        }

        if (!CreateShaders())
        {
            LOG_ERROR("Failed to create shaders");
            return false;
        }

        if (!CreateSampler())
        {
            LOG_ERROR("Failed to create sampler");
            return false;
        }

        m_capturer = std::make_unique<capturer::WGCCapturer>(m_graphicsDevice);
        capturer::WGCCapturer::CaptureDesc captureDesc{};
        captureDesc.frameRate = 60;
        captureDesc.format = graphics::TextureFormat::BGRA8_UNorm;
        captureDesc.captureCursor = true;
        captureDesc.captureBorder = true;

        if (!m_capturer->Initialize(captureDesc))
        {
            LOG_ERROR("Failed to initialize capturer");
            return false;
        }

        // 列出可用窗口
        auto windows = capturer::WGCCapturer::EnumerateWindows();
        LOG_INFO("Found {} windows", windows.size());

        for (size_t i = 0; i < windows.size() && i < 10; ++i)
        {
            std::wstring title(windows[i].windowTitle);
            LOG_INFO("  [{}] {}", i, winrt::to_string(title));
        }

        if (!windows.empty())
        {
            // 不要捕获自己的窗口，避免递归捕获导致闪烁
            HWND captureWindow = nullptr;
            for (size_t i = 0; i < windows.size(); ++i)
            {
                // 跳过自己的窗口
                if (windows[i].windowHandle != m_hwnd)
                {
                    captureWindow = windows[i].windowHandle;
                    LOG_INFO("Capturing window [{}]: {}", i, winrt::to_string(windows[i].windowTitle));
                    break;
                }
            }

            if (captureWindow && m_capturer->StartCapture(captureWindow))
            {
                LOG_INFO("Capture started successfully");
            }
            else
            {
                LOG_WARN("Failed to start capture or no suitable window found");
            }
        }
        else
        {
            LOG_WARN("No windows found to capture");
        }

        m_isInitialized = true;
        LOG_INFO("WGCTestApp initialized successfully");
        return true;
    }

    bool WGCTestApp::CreateRenderWindow(int width, int height)
    {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProcProxy;
        wc.hInstance = m_hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = m_className.c_str();

        if (!RegisterClassEx(&wc))
        {
            LOG_ERROR("Failed to register window class");
            return false;
        }

        RECT rect = { 0, 0, width, height };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hwnd = CreateWindowEx(
            0,
            m_className.c_str(),
            m_title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            m_hInstance,
            this);

        if (!m_hwnd)
        {
            LOG_ERROR("Failed to create window");
            return false;
        }

        ShowWindow(m_hwnd, m_nCmdShow);
        UpdateWindow(m_hwnd);

        LOG_INFO("Render window created: {}x{}", width, height);
        return true;
    }

    bool WGCTestApp::CreateShaders()
    {
        // 内联简单的着色器代码用于测试
        const char* vsCode = R"(
            float4 main(uint vertexID : SV_VertexID) : SV_Position
            {
                float2 positions[3] = {
                    float2(-1.0, -1.0),
                    float2(-1.0,  3.0),
                    float2( 3.0, -1.0)
                };
                return float4(positions[vertexID], 0.0, 1.0);
            }
        )";

        const char* psCode = R"(
            Texture2D tex0 : register(t0);
            SamplerState sampler0 : register(s0);

            float4 main(float4 position : SV_Position) : SV_Target0
            {
                // 将屏幕像素坐标归一化到[0,1]
                float2 texCoord = position.xy / float2(1280.0, 720.0);

                // 采样纹理并返回
                float4 color = tex0.Sample(sampler0, texCoord);
                return color;
            }
        )";

        // 编译顶点着色器
        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
        if (!CompileShader(vsCode, "main", "vs_5_0", vsBlob))
        {
            LOG_ERROR("Failed to compile vertex shader");
            return false;
        }

        if (!m_shader.LoadVertexShaderFromBytecode(m_graphicsDevice, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()))
        {
            LOG_ERROR("Failed to load vertex shader from bytecode");
            return false;
        }

        // 编译像素着色器
        Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
        if (!CompileShader(psCode, "main", "ps_5_0", psBlob))
        {
            LOG_ERROR("Failed to compile pixel shader");
            return false;
        }

        if (!m_shader.LoadPixelShaderFromBytecode(m_graphicsDevice, psBlob->GetBufferPointer(), psBlob->GetBufferSize()))
        {
            LOG_ERROR("Failed to load pixel shader from bytecode");
            return false;
        }

        LOG_INFO("Shaders loaded successfully");
        return true;
    }

    bool WGCTestApp::CompileShader(const std::string& code, const char* entryPoint, const char* target, Microsoft::WRL::ComPtr<ID3DBlob>& blob)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3DCompile(
            code.c_str(),
            code.size(),
            nullptr,
            nullptr,
            nullptr,
            entryPoint,
            target,
            D3DCOMPILE_ENABLE_STRICTNESS,
            0,
            &blob,
            &errorBlob
        );

        if (FAILED(hr))
        {
            if (errorBlob)
            {
                LOG_ERROR("Shader compilation error: {}", static_cast<char*>(errorBlob->GetBufferPointer()));
            }
            else
            {
                LOG_ERROR("Shader compilation failed: 0x{:X}", hr);
            }
            return false;
        }

        return true;
    }

    bool WGCTestApp::CreateSampler()
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        HRESULT hr = m_graphicsDevice->GetDevice()->CreateSamplerState(&samplerDesc, &m_sampler);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create sampler state: 0x{:X}", hr);
            return false;
        }

        LOG_INFO("Sampler created successfully");
        return true;
    }

    int WGCTestApp::Run()
    {
        if (!m_isInitialized)
        {
            LOG_ERROR("Application not initialized");
            return 1;
        }

        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                RenderFrame();
            }
        }

        return static_cast<int>(msg.wParam);
    }

    void WGCTestApp::RenderFrame()
    {
        m_graphicsDevice->BeginFrame();

        auto context = m_graphicsDevice->GetContext();

        // 清空背景为深灰色
        float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        context->ClearRenderTargetView(
            m_graphicsDevice->GetRenderTargetView(), clearColor);
        context->ClearDepthStencilView(
            m_graphicsDevice->GetDepthStencilView(),
            D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
            1.0f, 0);

        // 设置视口
        D3D11_VIEWPORT vp = {};
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = static_cast<float>(m_width);
        vp.Height = static_cast<float>(m_height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        context->RSSetViewports(1, &vp);

        // 获取最新帧并渲染
        static std::shared_ptr<graphics::Texture> lastFrame;

        if (m_capturer && m_capturer->HasNewFrame())
        {
            auto frame = m_capturer->GetLatestFrame();
            if (frame)
            {
                lastFrame = frame;
                RenderTexture(frame);
            }
        }
        else if (lastFrame)
        {
            // 没有新帧时，继续渲染上一帧
            RenderTexture(lastFrame);
        }

        m_graphicsDevice->EndFrame();
        m_graphicsDevice->Present(true);
    }

    void WGCTestApp::RenderTexture(std::shared_ptr<graphics::Texture> texture)
    {
        if (!texture || !texture->GetSRV())
        {
            return;
        }

        auto context = m_graphicsDevice->GetContext();

        // 设置着色器
        m_shader.SetToPipeline(m_graphicsDevice);
        context->IASetInputLayout(nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // 设置纹理和采样器
        ID3D11ShaderResourceView* srv[] = { texture->GetSRV() };
        context->PSSetShaderResources(0, 1, srv);
        context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

        // 绘制全屏三角形
        context->Draw(3, 0);
    }

    LRESULT CALLBACK WGCTestApp::WindowProcProxy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (uMsg == WM_CREATE)
        {
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            return 0;
        }

        WGCTestApp* app = reinterpret_cast<WGCTestApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (app)
        {
            return app->WndProc(hwnd, uMsg, wParam, lParam);
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    LRESULT CALLBACK WGCTestApp::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED && m_graphicsDevice)
            {
                uint32_t width = LOWORD(lParam);
                uint32_t height = HIWORD(lParam);
                if (width > 0 && height > 0)
                {
                    m_width = width;
                    m_height = height;
                    m_graphicsDevice->Resize(width, height);
                }
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}
