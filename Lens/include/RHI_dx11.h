#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <wrl/client.h>

namespace lens
{
    namespace
    {
        class Texture;
        class Shader;
    }

    class RHI_dx11
    {
    public:
        static RHI_dx11* Create(HWND hwnd, uint32_t width, uint32_t height);
        

        void Initialize(HWND hwnd, uint32_t width, uint32_t height);
        void Resize(uint32_t width, uint32_t height);
        ~RHI_dx11();

        void BeginFrame();
        void EndFrame();
        void Present(bool vsync);

        void Clear(const float clearColor[4]);

        void SetDefaultRenderTarget();
        //
        //void SetViewport(float width, float height);
        //void SetClearColor(float r, float g, float b, float a);
        //
        //void SetRenderTarget(class Texture* texture); // nullptr表示默认后备缓冲区
        //void SetDepthStencil(class Texture* texture); // nullptr表示禁用
        //
        //// 资源绑定
        //void BindShader(class Shader* shader);
        //void BindVertexBuffer(class Buffer* buffer, uint32_t slot = 0);
        //void BindIndexBuffer(class Buffer* buffer);
        //void BindConstantBuffer(class Buffer* buffer, uint32_t slot);
        //void BindTexture(class Texture* texture, uint32_t slot);
        //
        //// 绘制命令
        //void Draw(uint32_t vertexCount, uint32_t startVertex = 0);
        //void DrawIndexed(uint32_t indexCount, uint32_t startIndex = 0, int32_t baseVertex = 0);
        //
        //// 计算着色器
        //void Dispatch(uint32_t x, uint32_t y, uint32_t z);
        //
        //// 资源创建
        //std::shared_ptr<class Shader> CreateShader(const std::wstring& vsPath, const std::wstring& psPath);
        //std::shared_ptr<class Texture> CreateTexture(const std::wstring& path);
        //std::shared_ptr<class Buffer> CreateBuffer(const D3D11_BUFFER_DESC& desc, const D3D11_SUBRESOURCE_DATA* initData = nullptr);

        ID3D11Device* GetDevice() const { return device.Get(); }
        ID3D11DeviceContext* GetContext() const { return m_context.Get(); }
        IDXGISwapChain* GetSwapChain() const { return m_swapChain.Get(); }
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> defaultRTV = nullptr;
    private:
        RHI_dx11() = default;

        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        
        ID3D11DepthStencilView* m_defaultDSV = nullptr;
    };
}


