#include "LensPch.h"
#include "graphics/GraphicsDevice.h"
#include "Log.h"

namespace lens::graphics
{
    bool GraphicsDevice::Initialize(const Desc& desc) {
        // 创建设备和交换链
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        {
            swapChainDesc.BufferCount = 2;
            swapChainDesc.BufferDesc.Width = desc.width;
            swapChainDesc.BufferDesc.Height = desc.height;
            swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
            swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.OutputWindow = static_cast<HWND>(desc.windowHandle);
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Windowed = TRUE;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        }

        UINT createDeviceFlags = 0;
        if (desc.enableDebug) 
        {
            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        }

        D3D_FEATURE_LEVEL featureLevels[] = 
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
            featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
            &swapChainDesc, &m_swapChain, &m_device, nullptr, &m_context
        );
        if (FAILED(hr)) 
        {
            // TODO: 解析hr，给一个详细报错信息
            LOG_ERROR("Failed to create D3D11 device and swap chain, reason: {}", hr);
            return false;
        }

        // 创建渲染目标视图
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
        if (FAILED(hr)) 
        {
            LOG_ERROR("Failed to get back buffer from swap chain, reason: {}", hr);
            return false;
        }

        hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
        if (FAILED(hr)) 
        {
            LOG_ERROR("Failed to create render target view, reason: {}", hr);
            return false;
        }

        // 创建深度缓冲区
        D3D11_TEXTURE2D_DESC depthDesc = {};
        {
            depthDesc.Width = desc.width;
            depthDesc.Height = desc.height;
            depthDesc.MipLevels = 1;
            depthDesc.ArraySize = 1;
            depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthDesc.SampleDesc.Count = 1;
            depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        }

        hr = m_device->CreateTexture2D(&depthDesc, nullptr, &m_depthStencilBuffer);
        if (FAILED(hr)) 
        {
            LOG_ERROR("Failed to create depth stencil buffer, reason: {}", hr);
            return false;
        }

        hr = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView);
        if (FAILED(hr)) 
        {
            LOG_ERROR("Failed to create depth stencil view, reason: {}", hr);
            return false;
        }

        // 设置默认视口
        SetViewport(0.0f, 0.0f, static_cast<float>(desc.width), static_cast<float>(desc.height));

        return true;
    }

    void GraphicsDevice::BeginFrame()
    {
        float clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
        m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
        m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
        m_context->RSSetViewports(1, &m_viewport);
    }

    void GraphicsDevice::EndFrame()
    {

    }

    void GraphicsDevice::Present(bool vsync)
    {
        m_swapChain->Present(vsync ? 1 : 0, 0);
    }

    void GraphicsDevice::Resize(uint32_t width, uint32_t height)
    {
        // 释放现有的渲染目标视图
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_renderTargetView.Reset();
        m_depthStencilView.Reset();
        m_depthStencilBuffer.Reset();

        // 调整交换链大小
        HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr)) 
        {
            LOG_ERROR("Failed to resize swap chain buffers, reason: {}", hr);
            return;
        }

        // 重新创建渲染目标视图
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
        if (SUCCEEDED(hr)) 
        {
            m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
        }

        // 重新创建深度缓冲区
        D3D11_TEXTURE2D_DESC depthDesc = {};
        {
            depthDesc.Width = width;
            depthDesc.Height = height;
            depthDesc.MipLevels = 1;
            depthDesc.ArraySize = 1;
            depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthDesc.SampleDesc.Count = 1;
            depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        }

        m_device->CreateTexture2D(&depthDesc, nullptr, &m_depthStencilBuffer);
        m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView);

        // 更新视口
        SetViewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    }

    void GraphicsDevice::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
    {
        m_viewport.TopLeftX = x;
        m_viewport.TopLeftY = y;
        m_viewport.Width = width;
        m_viewport.Height = height;
        m_viewport.MinDepth = minDepth;
        m_viewport.MaxDepth = maxDepth;
    }

    void GraphicsDevice::SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
    {
        D3D11_RECT scissorRect;
        scissorRect.left = static_cast<LONG>(left);
        scissorRect.top = static_cast<LONG>(top);
        scissorRect.right = static_cast<LONG>(right);
        scissorRect.bottom = static_cast<LONG>(bottom);
        m_context->RSSetScissorRects(1, &scissorRect);
    }

    void GraphicsDevice::SetVertexShader(ID3D11VertexShader* shader)
    {
        m_context->VSSetShader(shader, nullptr, 0);
    }

    void GraphicsDevice::SetPixelShader(ID3D11PixelShader* shader)
    {
        m_context->PSSetShader(shader, nullptr, 0);
    }

    void GraphicsDevice::SetComputeShader(ID3D11ComputeShader* shader)
    {
        m_context->CSSetShader(shader, nullptr, 0);
    }

    void GraphicsDevice::SetInputLayout(ID3D11InputLayout* layout)
    {
        m_context->IASetInputLayout(layout);
    }
}
