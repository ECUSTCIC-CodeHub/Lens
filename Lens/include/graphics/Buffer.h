#pragma once

#include "GraphicsDevice.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace lens::graphics 
{

    using Microsoft::WRL::ComPtr;

    enum class BufferType 
    {
        Vertex = D3D11_BIND_VERTEX_BUFFER,
        Index = D3D11_BIND_INDEX_BUFFER,
        Constant = D3D11_BIND_CONSTANT_BUFFER,
        ShaderResource = D3D11_BIND_SHADER_RESOURCE,
        UnorderedAccess = D3D11_BIND_UNORDERED_ACCESS
    };

    class Buffer 
    {
    public:
        struct Desc 
        {
            size_t size = 0;
            BufferType type = BufferType::Vertex;
            BufferUsage usage = BufferUsage::Static;
            uint32_t structureStride = 0;  // 用于结构化缓冲区
            bool isUAV = false;
        };

        Buffer() = default;
        ~Buffer() = default;

        bool Create(GraphicsDevice* device, const Desc& desc, const void* initialData = nullptr);

        // 直接访问
        ID3D11Buffer* GetD3DBuffer() const { return m_buffer.Get(); }
        ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }
        ID3D11UnorderedAccessView* GetUAV() const { return m_uav.Get(); }

        // 属性
        const Desc& GetDesc() const { return m_desc; }
        size_t GetSize() const { return m_desc.size; }

        // 数据操作
        void UpdateData(GraphicsDevice* device, const void* data, size_t size, size_t offset = 0);
        D3D11_MAPPED_SUBRESOURCE Map(GraphicsDevice* device, D3D11_MAP mapType = D3D11_MAP_WRITE_DISCARD);
        void Unmap(GraphicsDevice* device);

        // 模板辅助方法
        template<typename T>
        void UpdateStruct(GraphicsDevice* device, const T& data) 
        {
            UpdateData(device, &data, sizeof(T));
        }

        template<typename T>
        T* MapAs(GraphicsDevice* device) 
        {
            return static_cast<T*>(Map(device).pData);
        }

    private:
        ComPtr<ID3D11Buffer> m_buffer;
        ComPtr<ID3D11ShaderResourceView> m_srv;
        ComPtr<ID3D11UnorderedAccessView> m_uav;

        Desc m_desc{};
    };

}