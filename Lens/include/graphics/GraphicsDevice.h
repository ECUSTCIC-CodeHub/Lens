#pragma once

#include <cstdint>
#include <span>
#include <memory>
#include <vector>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>


namespace lens::graphics
{

    enum class TextureFormat 
    {
        RGBA8_UNorm     = DXGI_FORMAT_R8G8B8A8_UNORM,
        BGRA8_UNorm     = DXGI_FORMAT_B8G8R8A8_UNORM,
        R32_Float       = DXGI_FORMAT_R32_FLOAT,
        RG32_Float      = DXGI_FORMAT_R32G32_FLOAT,
        RGBA32_Float    = DXGI_FORMAT_R32G32B32A32_FLOAT,
        R16_UInt        = DXGI_FORMAT_R16_UINT,
        D24_UNorm_S8_UInt = DXGI_FORMAT_D24_UNORM_S8_UINT
    };

    enum class BufferUsage 
    {
        Static  = D3D11_USAGE_DEFAULT,      // GPU只读，CPU一次性写入
        Dynamic = D3D11_USAGE_DYNAMIC,     // CPU读写，GPU只读
        Staging = D3D11_USAGE_STAGING      // CPU读写，GPU访问禁用
    };

    class GraphicsDevice 
    {
    public:
        struct Desc 
        {
            void* windowHandle = nullptr;
            uint32_t width = 1920;
            uint32_t height = 1080;
            bool enableDebug = false;
        };

        GraphicsDevice() = default;
        ~GraphicsDevice() = default;

        bool Initialize(const Desc& desc);

        // 帧控制
        void BeginFrame();
        void EndFrame();
        void Present(bool vsync = true);
        void Resize(uint32_t width, uint32_t height);

        // 直接访问对象
        ID3D11Device* GetDevice() const { return m_device.Get(); }
        ID3D11DeviceContext* GetContext() const { return m_context.Get(); }
        IDXGISwapChain* GetSwapChain() const { return m_swapChain.Get(); }

        // 渲染资源
        ID3D11RenderTargetView* GetRenderTargetView() const { return m_renderTargetView.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthStencilView.Get(); }

        // 视口操作
        void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
        void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);

        // 着色器设置
        void SetVertexShader(ID3D11VertexShader* shader);
        void SetPixelShader(ID3D11PixelShader* shader);
        void SetComputeShader(ID3D11ComputeShader* shader);
        void SetInputLayout(ID3D11InputLayout* layout);

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;

        D3D11_VIEWPORT m_viewport{};
    };

}
