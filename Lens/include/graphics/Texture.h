#pragma once

#include "GraphicsDevice.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace lens::graphics 
{
    enum class TextureType 
    {
        Texture1D,
        Texture2D,
        Texture3D
    };

    class Texture 
    {
    public:
        struct Desc 
        {
            uint32_t width  = 1;
            uint32_t height = 1;
            uint32_t depth  = 1;
            uint32_t arraySize   = 1;
            uint32_t mipLevels   = 1;
            uint32_t sampleCount = 1;
            TextureFormat format = TextureFormat::RGBA8_UNorm;
            TextureType type     = TextureType::Texture2D;
            bool bindRenderTarget    = false;
            bool bindShaderResource  = true;
            bool bindUnorderedAccess = false;
        };

        Texture() = default;
        ~Texture() = default;

        // 创建方法
        bool Create(GraphicsDevice* device, const Desc& desc);
        bool CreateFromMemory(GraphicsDevice* device, const Desc& desc, const void* data);
        bool CreateFromD3DTexture(GraphicsDevice* device, ID3D11Texture2D* texture);

        // 直接访问 D3D11 资源
        ID3D11Texture2D* GetD3DTexture() const { return m_texture.Get(); }
        ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }
        ID3D11RenderTargetView* GetRTV() const { return m_rtv.Get(); }
        ID3D11UnorderedAccessView* GetUAV() const { return m_uav.Get(); }

        // 属性访问
        const Desc& GetDesc() const { return m_desc; }
        uint32_t GetWidth() const { return m_desc.width; }
        uint32_t GetHeight() const { return m_desc.height; }
        TextureFormat GetFormat() const { return m_desc.format; }

        // 数据操作
        void UpdateData(GraphicsDevice* device, const void* data, size_t size, uint32_t mipLevel = 0);
        D3D11_MAPPED_SUBRESOURCE Map(GraphicsDevice* device, uint32_t mipLevel = 0, D3D11_MAP mapType = D3D11_MAP_WRITE_DISCARD);
        void Unmap(GraphicsDevice* device, uint32_t mipLevel = 0);

        // Mipmap 生成
        void GenerateMipmaps(GraphicsDevice* device);

    private:
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_uav;

        Desc m_desc{};
    };

}