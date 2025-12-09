#pragma once

#include "LensPch.h"
#include <memory>
#include <vector>
#include "RHI_dx11.h"

namespace lens::graphics
{
    /**
     * 图形资源接口
     */
    class IGraphicsResource
    {
    public:
        virtual ~IGraphicsResource() = default;
        virtual void Release() = 0;
        virtual bool IsValid() const = 0;
    };

    /**
     * 图形设备抽象
     */
    class IGraphicsDevice
    {
    public:
        virtual ~IGraphicsDevice() = default;
        virtual ID3D11Device* GetDevice() const = 0;
        virtual ID3D11DeviceContext* GetContext() const = 0;
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Present() = 0;
        virtual void Clear(const float color[4]) = 0;
        virtual std::shared_ptr<ITexture> CreateTexture(uint32_t width, uint32_t height, DXGI_FORMAT format) = 0;
        virtual std::shared_ptr<IBuffer> CreateBuffer(uint32_t size, uint32_t stride) = 0;
        virtual std::shared_ptr<IFrameBuffer> CreateFrameBuffer(uint32_t width, uint32_t height, DXGI_FORMAT format) = 0;
        virtual std::shared_ptr<IShader> CreateShader(const std::vector<uint8_t>& vsCode, const std::vector<uint8_t>& psCode, const std::vector<InputElement>& inputLayout) = 0;
    };

    /**
     * 纹理接口
     */
    class IBuffer : public IGraphicsResource
    {
    public:
        virtual ~IBuffer() = default;
        virtual ID3D11Buffer* GetBuffer() const = 0;
        virtual void* Map() = 0;
        virtual void Unmap() = 0;
        virtual uint32_t GetSize() const = 0;
        virtual uint32_t GetStride() const = 0;
        virtual void Bind(ID3D11DeviceContext* context, uint32_t slot = 0) = 0;
        virtual ID3D11ShaderResourceView* GetSRV() const = 0;
    };

    /**
     * 纹理体类型接口
     */
    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT2 texCoord;
    };

    /**
     * 常顶索引接口
     */
    struct Index
    {
        uint32_t value;
    };

    /**
     * 常量缓冲区接口
     */
    struct ConstantBuffer
    {
        alignas(16) uint32_t data[64];
    };

    /**
     * 着色器接口
     */
    class IShader : public IGraphicsResource
    {
    public:
        virtual ~IShader() = default;
        virtual ID3D11VertexShader* GetVS() const = 0;
        virtual ID3D11PixelShader* GetPS() const = 0;
        virtual ID3D11InputLayout* GetInputLayout() const = 0;
        virtual ID3D11Device* GetDevice() const = 0;
    };

    /**
     * 帧区目标接口
     */
    class IRenderTarget : public IGraphicsResource
    {
    public:
        virtual ~IRenderTarget() = default;
        virtual void Bind(ID3D11DeviceContext* context, uint32_t slot = 0) = 0;
        virtual ID3D11RenderTargetView* GetRTV() const = 0;
        virtual ID3D11DepthStencilView* GetDSV() const = 0;
        virtual void Clear(const float color[4]) = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
    };

    /**
     * 纹理管目标接口
     */
    class IPipeline
    {
    public:
        virtual ~IPipeline() = default;
        virtual void Bind() = 0;
        virtual void SetShader(const std::shared_ptr<IShader>& shader) = 0;
        virtual void SetInputLayout(const std::shared_ptr<IBuffer>& vertexBuffer, uint32_t stride) = 0;
        virtual void SetVertexShader(const std::shared_ptr<IBuffer>& vertexBuffer) = 0;
        virtual void SetIndexBuffer(const std::shared_ptr<IBuffer>& indexBuffer) = 0;
        virtual void SetConstantBuffer(uint32_t slot, const std::shared_ptr<IBuffer>& constantBuffer) = 0;
        virtual void SetRenderTarget(const std::shared_ptr<IRenderTarget>& renderTarget) = 0;
        virtual void SetViewport(const float x, const float y, const float width, const float height, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
        virtual void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology) = 0;
        virtual void Draw(uint32_t vertexCount, uint32_t startVertex = 0) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndex = 0, int32_t baseVertex = 0) = 0;
    };

} // namespace lens::graphics