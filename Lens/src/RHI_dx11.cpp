#include "LensPch.h"
#include "RHI_dx11.h"

namespace lens
{
    RHI_dx11* RHI_dx11::Create(HWND hwnd, uint32_t width, uint32_t height)
    {
        return new RHI_dx11;
    }

    void RHI_dx11::Initialize(HWND hwnd, uint32_t width, uint32_t height)
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        {
            swapChainDesc.BufferCount = 2;
            swapChainDesc.BufferDesc.Width = 0;
            swapChainDesc.BufferDesc.Height = 0;
            swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
            swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
            swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.OutputWindow = hwnd;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Windowed = TRUE;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        }

        UINT createDeviceFlags = 0;

        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

        // 创建设备和交换链
        D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &m_swapChain,
            &device,
            &featureLevel,
            &m_context);

        ID3D11Texture2D* pBackBuffer;
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        device->CreateRenderTargetView(pBackBuffer, nullptr, &defaultRTV);
        pBackBuffer->Release();
    }

    void RHI_dx11::Resize(uint32_t width, uint32_t height)
    {
        if (!m_swapChain || !device || !m_context)
            return;

        // 释放现有的渲染目标视图
        if (defaultRTV)
        {
            m_context->OMSetRenderTargets(0, nullptr, nullptr);
            defaultRTV->Release();
            defaultRTV = nullptr;
        }

        // 调整交换链大小
        HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to resize swap chain buffers");
            return;
        }

        // 创建新的渲染目标视图
        ID3D11Texture2D* pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to get back buffer after resize");
            return;
        }

        hr = device->CreateRenderTargetView(pBackBuffer, nullptr, &defaultRTV);
        pBackBuffer->Release();

        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create render target view after resize");
            return;
        }

        // 设置新的视口
        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        m_context->RSSetViewports(1, &viewport);

        LOG_INFO("RHI resized to {}x{}", width, height);
    }

    RHI_dx11::~RHI_dx11()
    {
        if (m_context) m_context->ClearState();
        if (defaultRTV) defaultRTV->Release();
        if (m_defaultDSV) m_defaultDSV->Release();
        if (m_context) m_context->Release();
        if (m_swapChain) m_swapChain->Release();
        if (device) device->Release();
    }
}