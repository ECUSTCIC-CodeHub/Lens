#include "LensPch.h"
#include "graphics/Buffer.h"
#include "Log.h"

namespace lens::graphics 
{

    bool Buffer::Create(GraphicsDevice* device, const Desc& desc, const void* initialData) 
    {
        m_desc = desc;

        D3D11_BUFFER_DESC bufferDesc = {};
        {
            bufferDesc.ByteWidth = static_cast<UINT>(desc.size);
            bufferDesc.Usage = static_cast<D3D11_USAGE>(desc.usage);
            bufferDesc.BindFlags = static_cast<UINT>(desc.type);
            // 根据用法设置CPU访问标志
            if (desc.usage == BufferUsage::Dynamic) 
            {
                bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            }
            else if (desc.usage == BufferUsage::Staging) 
            {
                bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            }
            else
            {
                bufferDesc.CPUAccessFlags = 0;
            }
            bufferDesc.MiscFlags = 0;
            bufferDesc.StructureByteStride = desc.structureStride;
        }

        D3D11_SUBRESOURCE_DATA* initData = nullptr;
        D3D11_SUBRESOURCE_DATA subresourceData = {};
        if (initialData) 
        {
            subresourceData.pSysMem = initialData;
            initData = &subresourceData;
        }

        HRESULT hr = device->GetDevice()->CreateBuffer(&bufferDesc, initData, &m_buffer);
        if (FAILED(hr)) 
        {
            return false;
        }

        // 创建着色器资源视图（如果是结构化缓冲区）
        if (desc.type == BufferType::ShaderResource && desc.structureStride > 0) 
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = static_cast<UINT>(desc.size / desc.structureStride);

            hr = device->GetDevice()->CreateShaderResourceView(m_buffer.Get(), &srvDesc, &m_srv);
            if (FAILED(hr)) {
                return false;
            }
        }

        // 创建无序访问视图
        if (desc.isUAV && desc.structureStride > 0) 
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = static_cast<UINT>(desc.size / desc.structureStride);
            uavDesc.Buffer.Flags = 0;

            hr = device->GetDevice()->CreateUnorderedAccessView(m_buffer.Get(), &uavDesc, &m_uav);
            if (FAILED(hr)) 
            {
                return false;
            }
        }

        return true;
    }

    void Buffer::UpdateData(GraphicsDevice* device, const void* data, size_t size, size_t offset) 
    {
        if (m_desc.usage == BufferUsage::Dynamic) 
        {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT hr = device->GetContext()->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (SUCCEEDED(hr)) 
            {
                memcpy(static_cast<char*>(mappedResource.pData) + offset, data, size);
                device->GetContext()->Unmap(m_buffer.Get(), 0);
            }
        }
        else 
        {
            D3D11_BOX box = {};
            box.left = static_cast<UINT>(offset);
            box.right = static_cast<UINT>(offset + size);
            box.top = 0;
            box.bottom = 1;
            box.front = 0;
            box.back = 1;

            device->GetContext()->UpdateSubresource(m_buffer.Get(), 0, &box, data, 0, 0);
        }
    }

    D3D11_MAPPED_SUBRESOURCE Buffer::Map(GraphicsDevice* device, D3D11_MAP mapType) 
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = device->GetContext()->Map(m_buffer.Get(), 0, mapType, 0, &mappedResource);

        if (FAILED(hr)) 
        {
            return {};
        }

        return mappedResource;
    }

    void Buffer::Unmap(GraphicsDevice* device) 
    {
        device->GetContext()->Unmap(m_buffer.Get(), 0);
    }

}