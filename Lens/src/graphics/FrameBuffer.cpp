#include "LensPch.h"
#include "graphics/FrameBuffer.h"
#include "RHI_dx11.h"
#include "Log.h"

namespace lens::graphics
{
    FrameBuffer::FrameBuffer(ID3D11Buffer* buffer, Type type, uint32_t width, uint32_t height,
        Usage usage, DXGI_FORMAT format)
        : m_buffer(buffer)
        , m_width(width)
        , m_height(height)
        , m_type(type)
        , m_usage(usage)
        , m_format(format)
    {
    }

    
    std::shared_ptr<FrameBuffer> FrameBuffer::Create(
        ID3D11Device* device,
        uint32_t width, uint32_t height,
        Type type,
        Usage usage,
        DXGI_FORMAT format)
    {
        if (!device || width == 0 || height == 0) {
            LOG_ERROR("Invalid parameters for frame buffer creation");
            return nullptr;
        }

        // 创建D3D11缓冲区描述
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = 0;

        // 根据类型设置不同的标志
        switch (type) {
            case Type::Index:
                desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                break;
            case Type::RenderTarget:
                desc.BindFlags = D3D11_BIND_RENDER_TARGET;
                break;
            case Type::DepthStencil:
                desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
                break;
            case Type::Structured:
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.StructureByteStride = 16; // 假设16字节结构体
                desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
                break;
            case Type::Constant:
                desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                desc.Usage = D3D11_USAGE_DYNAMIC;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                desc.ByteWidth = ((sizeof(uint32_t) * 4 + 15) & ~15); // 16字节对齐
                break;
            default:
                break;
        }

        // 对于非常量缓冲区，设置适当的大小
        if (type != Type::Constant) {
            desc.ByteWidth = width * height * 4; // 假设RGBA8格式
        }

        // 创建D3D11缓冲区
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        HRESULT hr = device->CreateBuffer(&desc, nullptr, &buffer);

        if (FAILED(hr)) {
            LOG_ERROR("Failed to create frame buffer: 0x{:08X}", hr);
            return nullptr;
        }

        auto frameBuffer = std::make_shared<FrameBuffer>(buffer.Get(), type, width, height, usage, format);

        LOG_DEBUG("FrameBuffer created: type={} size={}x{} format={}",
                  static_cast<int>(type), width, height, static_cast<uint32_t>(format));
        return frameBuffer;
    }

    void FrameBuffer::Bind(ID3D11DeviceContext* context, uint32_t slot)
    {
        if (!context || !m_buffer) {
            LOG_ERROR("Invalid context or buffer for binding");
            return;
        }

        UINT offset = 0;
        UINT stride = 0;

        switch (m_type) {
            case Type::Index:
                context->IASetIndexBuffer(m_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
                break;
            case Type::Constant:
                context->CSSetConstantBuffers(slot, 1, &m_buffer);
                context->VSSetConstantBuffers(slot, 1, &m_buffer);
                context->PSSetConstantBuffers(slot, 1, &m_buffer);
                break;
            case Type::Structured:
                stride = 16; // 结构体步长
                context->IASetVertexBuffers(slot, 1, &m_buffer, &stride, &offset);
                break;
            default:
                // 顶点缓冲区默认绑定
                stride = sizeof(float) * 4; // 假设position + color
                context->IASetVertexBuffers(slot, 1, &m_buffer, &stride, &offset);
                break;
        }

        if (m_type == Type::Index || m_type == Type::Structured) {
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }
    }

    uint32_t FrameBuffer::GetSize() const
    {
        return m_width * m_height * 4; // 假设RGBA8格式
    }

    uint32_t FrameBuffer::GetStride() const
    {
        switch (m_type) {
            case Type::Constant:
                return 16; // 常量缓冲区16字节对齐
            case Type::Structured:
                return 16; // 结构化缓冲区假设16字节
            case Type::Index:
                return sizeof(uint32_t); // 32位索引
            default:
                return sizeof(float) * 4; // 默认顶点格式
        }
    }

    bool FrameBuffer::IsValid() const
    {
        return m_buffer != nullptr;
    }

    FrameBuffer::Type FrameBuffer::GetType() const
    {
        return m_type;
    }

    ID3D11ShaderResourceView* FrameBuffer::GetSRV() const
    {
        return m_srv.Get();
    }

    ID3D11RenderTargetView* FrameBuffer::GetRTV() const
    {
        return m_rtv.Get();
    }

    ID3D11DepthStencilView* FrameBuffer::GetDSV() const
    {
        return m_dsv.Get();
    }

    void FrameBuffer::Map(void** mappedData)
    {
        if (!m_buffer || !mappedData) {
            LOG_ERROR("Cannot map buffer: buffer is null or no mapped data provided");
            return;
        }

        // 获取设备上下文 - 这里简化处理，实际应该从外部传入
        // 由于RHI_dx11没有提供从buffer获取context的方法，这里先返回空
        LOG_WARN("FrameBuffer::Map not fully implemented - need device context");
        *mappedData = nullptr;
    }

    void FrameBuffer::Unmap()
    {
        // 简化实现，需要设备上下文
        LOG_WARN("FrameBuffer::Unmap not fully implemented - need device context");
    }

} // namespace lens::graphics