#include "LensPch.h"
#include "Application.h"
#include "graphics/GraphicsDevice.h"
#include "ImguiManager.h"
#include "gui/DemoPanel.h"
#include "gui/DebugPanel.h"
#include "gui/CapturePanel.h"
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
        // 初始化device
        m_graphicsDevice = new graphics::GraphicsDevice();
        graphics::GraphicsDevice::Desc deviceDesc{};
        {
            deviceDesc.windowHandle = m_hwnd;
            deviceDesc.width = this->width;
            deviceDesc.height = this->height;
            deviceDesc.enableDebug = true;
        }

        if (!m_graphicsDevice->Initialize(deviceDesc))
        {
            LOG_ERROR("Failed to initialize graphics device");
            return;
        }

        // 初始化imgui
        m_imgui = new ImguiManager();
        m_imgui->Initialize(m_hwnd, m_graphicsDevice->GetDevice(), m_graphicsDevice->GetContext());

        // 置 ImGui 的 DisplaySize
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)this->width, (float)this->height);

        // 获取窗口实际 DPI 并设置缩放因子
        float dpiScale = GetDpiForWindow(m_hwnd) / 96.0f;
        io.DisplayFramebufferScale = ImVec2(dpiScale, dpiScale);

        LOG_INFO("ImGui display size: {}x{}, DPI scale: {:.2f}", this->width, this->height, dpiScale);

        m_imgui->SetMenuVisible(true);

        // 初始化UIManager
        UIManager* uiManager = m_imgui->GetUIManager();
        uiManager->InitializeAllPanels(nullptr);

        if (!CreateCaptureShaders())
        {
            LOG_ERROR("Failed to create capture shaders");
        }

        if (!CreateCaptureSampler())
        {
            LOG_ERROR("Failed to create capture sampler");
        }

        // 初始化Capturer，后续需要修改
        m_capturer = std::make_unique<capturer::WGCCapturer>(m_graphicsDevice);
        capturer::WGCCapturer::CaptureDesc captureDesc{};
        captureDesc.frameRate = 60;
        captureDesc.format = graphics::TextureFormat::BGRA8_UNorm;
        captureDesc.captureCursor = true;
        captureDesc.captureBorder = true;

        if (!m_capturer->Initialize(captureDesc))
        {
            LOG_ERROR("Failed to initialize capturer");
        }
        else
        {
            LOG_INFO("Capturer initialized successfully");

            // Capturer 初始化成功后，注册 CapturePanel
            auto* capturePanel = uiManager->AddPanel<CapturePanel>();
            if (capturePanel)
            {
                capturePanel->SetCapturer(m_capturer.get());
                capturePanel->SetVisible(true);
                LOG_INFO("CapturePanel registered and configured");
            }

            auto windows = capturer::WGCCapturer::EnumerateWindows();
            LOG_INFO("Found {} windows", windows.size());

            for (size_t i = 0; i < windows.size() && i < 10; ++i)
            {
                std::wstring title(windows[i].windowTitle);
                LOG_INFO("  [{}] {}", i, winrt::to_string(title));
            }

            if (!windows.empty())
            {
                HWND captureWindow = nullptr;
                for (size_t i = 0; i < windows.size(); ++i)
                {
                    if (windows[i].windowHandle != m_hwnd)
                    {
                        captureWindow = windows[i].windowHandle;
                        LOG_INFO("Auto-capturing window [{}]: {}", i, winrt::to_string(windows[i].windowTitle));
                        break;
                    }
                }

                if (captureWindow && m_capturer->StartCapture(captureWindow))
                {
                    LOG_INFO("Capture started successfully");
                }
            }
        }

        LOG_INFO("Application initialized successfully ");
    }

    int Application::Run()
    {
        MSG msg = { 0 };
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

            m_graphicsDevice->BeginFrame();

            m_imgui->BeginFrame();

            m_imgui->GetUIManager()->RenderAll();

            m_imgui->EndFrame();

            m_graphicsDevice->EndFrame();
            m_graphicsDevice->Present(true);
        }

        return static_cast<int>(msg.wParam);
    }

    // 窗口回调，算是标准写法，会转发到类内的函数处理
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
                    int newWidth = LOWORD(lParam);
                    int newHeight = HIWORD(lParam);

                    this->width = newWidth;
                    this->height = newHeight;

                    // 调整device
                    this->m_graphicsDevice->Resize(newWidth, newHeight);

                    // 调整imgui
                    ImGuiIO& io = ImGui::GetIO();
                    io.DisplaySize = ImVec2((float)newWidth, (float)newHeight);

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
        // 保存窗口尺寸
        this->width = width;
        this->height = height;

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

        // 获取 DPI 缩放后的实际窗口大小
        RECT rect;
        GetClientRect(m_hwnd, &rect);
        this->width = rect.right - rect.left;
        this->height = rect.bottom - rect.top;

        LOG_INFO("Window created successfully. Requested: {}x{}, Actual: {}x{}",
            width, height, this->width, this->height);
        return true;
    }

    // 临时使用的shader
    bool Application::CreateCaptureShaders()
    {
        // Simple fullscreen triangle shaders
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

            cbuffer CaptureParams : register(b0)
            {
                float2 WindowSize;
                float2 TextureSize;
            }

            float4 main(float4 position : SV_Position) : SV_Target0
            {
                // Normalize screen coordinates to [0,1]
                float2 texCoord = position.xy / WindowSize;

                // Sample texture and return
                float4 color = tex0.Sample(sampler0, texCoord);
                return color;
            }
        )";

        // Compile vertex shader
        Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
        if (!CompileShader(vsCode, "main", "vs_5_0", vsBlob))
        {
            LOG_ERROR("Failed to compile vertex shader");
            return false;
        }

        if (!m_captureShader.LoadVertexShaderFromBytecode(m_graphicsDevice, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize()))
        {
            LOG_ERROR("Failed to load vertex shader from bytecode");
            return false;
        }

        // Compile pixel shader
        Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
        if (!CompileShader(psCode, "main", "ps_5_0", psBlob))
        {
            LOG_ERROR("Failed to compile pixel shader");
            return false;
        }

        if (!m_captureShader.LoadPixelShaderFromBytecode(m_graphicsDevice, psBlob->GetBufferPointer(), psBlob->GetBufferSize()))
        {
            LOG_ERROR("Failed to load pixel shader from bytecode");
            return false;
        }

        // Create constant buffer
        D3D11_BUFFER_DESC cbDesc = {};
        cbDesc.ByteWidth = sizeof(float) * 4;  // 2 floats for WindowSize + 2 floats for TextureSize
        cbDesc.Usage = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;

        HRESULT hr = m_graphicsDevice->GetDevice()->CreateBuffer(&cbDesc, nullptr, &m_captureConstantBuffer);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create constant buffer: 0x{:X}", hr);
            return false;
        }

        LOG_INFO("Capture shaders loaded successfully");
        return true;
    }

    bool Application::CompileShader(const std::string& code, const char* entryPoint, const char* target, Microsoft::WRL::ComPtr<ID3DBlob>& blob)
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
    // 临时放这，后续调整
    bool Application::CreateCaptureSampler()
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

        HRESULT hr = m_graphicsDevice->GetDevice()->CreateSamplerState(&samplerDesc, &m_captureSampler);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create sampler state: 0x{:X}", hr);
            return false;
        }

        LOG_INFO("Capture sampler created successfully");
        return true;
    }
}
