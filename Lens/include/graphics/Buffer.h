#pragma once

#include "RHI_dx11.h"
#include <memory>

namespace lens::graphics
{
    // 缓冲区资源
    class Buffer 
    {
    public:
        enum class Type 
        {
            Vertex,
            Index,
            Constant,
            Structured
        };

        static std::shared_ptr<Buffer> Create(
            ID3D11Device* device,
            Type type,
            uint32_t size,
            uint32_t stride,
            const void* initData = nullptr);

        ID3D11Buffer* GetBuffer() const { return m_buffer; }
        uint32_t GetSize() const { return m_size; }
        uint32_t GetStride() const { return m_stride; }

    private:
        Buffer(ID3D11Buffer* buffer, uint32_t size, uint32_t stride);

        ID3D11Buffer* m_buffer;
        uint32_t m_size;
        uint32_t m_stride;
    };
}
