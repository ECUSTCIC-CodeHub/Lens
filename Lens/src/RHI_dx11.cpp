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
            &m_device,
            &featureLevel,
            &m_context);

        ID3D11Texture2D* pBackBuffer;
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_defaultRTV);
        pBackBuffer->Release();
    }

    RHI_dx11::~RHI_dx11()
    {
        if (m_context) m_context->ClearState();
        if (m_defaultRTV) m_defaultRTV->Release();
        if (m_defaultDSV) m_defaultDSV->Release();
        if (m_context) m_context->Release();
        if (m_swapChain) m_swapChain->Release();
        if (m_device) m_device->Release();
    }

    
}