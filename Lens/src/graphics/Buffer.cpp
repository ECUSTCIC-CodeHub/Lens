#include "LensPch.h"
#include "graphics/Buffer.h"
#include "RHI_dx11.h"
#include "Log.h"

namespace lens::graphics
{
    std::shared_ptr<Buffer> Buffer::Create(
        ID3D11Device* device,
        Type type,
        uint32_t size,
        uint32_t stride,
        const void* initData)
    {
        if (!device || size == 0) {
            LOG_ERROR("Invalid parameters for buffer creation");
            return nullptr;
        }

        auto buffer = std::make_shared<Buffer>();
        buffer->m_type = type;
        buffer->m_size = size;
        buffer->m_stride = stride;

        // 创建D3D11缓冲区描述
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = stride;
        desc.ByteWidth = stride * 8; // 假设每行8个字节（4个float或颜色）
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        // 根据缓冲区类型设置不同的标志
        switch (type) {
            case Type::Vertex:
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                break;
            case Type::Index:
                desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                break;
            case Type::Constant:
                desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                desc.Usage = D3D11_USAGE_DEFAULT;
                break;
            case Type::Structured:
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.StructureByteStride = stride;
                break;
        }

        // 创建子资源数据
        D3D11_SUBRESOURCE_DATA* subresourceData = nullptr;
        D3D11_SUBRESOURCE_DATA dataDesc = {};
        if (initData) {
            dataDesc.pSysMem = initData;
            dataDesc.SysMemPitch = stride;
            subresourceData = &dataDesc;
        }

        // 创建D3D11缓冲区
        HRESULT hr = device->CreateBuffer(&desc, subresourceData, &buffer->m_buffer);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create D3D11 buffer: 0x{:08X}", hr);
            return nullptr;
        }

        // 根据缓冲区类型创建额外的视图
        if (type == Type::Structured) {
            // 创建着色器资源视图
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_UNKNOWN; // 需要具体格式
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.ElementWidth = 1;
            srvDesc.Buffer.NumElements = size / stride;
            srvDesc.Buffer.FirstElement = 0;

            hr = device->CreateShaderResourceView(buffer->m_buffer.Get(), &srvDesc, &buffer->m_srv);
            if (FAILED(hr)) {
                LOG_ERROR("Failed to create shader resource view for structured buffer: 0x{:08X}", hr);
                return nullptr;
            }

            // 创建无序访问视图
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = srvDesc.Format;
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = size / stride;
            uavDesc.Buffer.Flags = 0;

            hr = device->CreateUnorderedAccessView(buffer->m_buffer.Get(), &uavDesc, &buffer->m_uav);
            if (FAILED(hr)) {
                LOG_ERROR("Failed to create unordered access view for structured buffer: 0x{:08X}", hr);
                return nullptr;
            }
        }

        LOG_DEBUG("Buffer created successfully: size={} stride={} type={}",
                  size, stride, static_cast<int>(type));
        return buffer;
    }

    void Buffer::UpdateData(const void* data, uint32_t size, uint32_t offset)
    {
        if (!m_buffer || !data) {
            LOG_ERROR("Cannot update buffer: invalid parameters");
            return;
        }

        // 简化实现，暂时不支持动态更新
        LOG_WARN("Buffer UpdateData not fully implemented - need device context");
    }

    
} // namespace lens::graphics