#include "LensPch.h"
#include "graphics/Texture.h"

namespace lens::graphics
{
    bool Texture::Create(GraphicsDevice* device, const Desc& desc) 
    {
        m_desc = desc;

        D3D11_TEXTURE2D_DESC texDesc = {};
        {
            texDesc.Width = desc.width;
            texDesc.Height = desc.height;
            texDesc.MipLevels = desc.mipLevels;
            texDesc.ArraySize = desc.arraySize;
            texDesc.Format = static_cast<DXGI_FORMAT>(desc.format);
            texDesc.SampleDesc.Count = desc.sampleCount;
            texDesc.BindFlags = 0;
            if (desc.bindShaderResource) 
            {
                texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
            }
            if (desc.bindRenderTarget) 
            {
                texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            }
            if (desc.bindUnorderedAccess) 
            {
                texDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
            }
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;
        }

        HRESULT hr = device->GetDevice()->CreateTexture2D(&texDesc, nullptr, &m_texture);
        if (FAILED(hr)) 
        {
            LOG_ERROR("Failed to create D3D11 texture");
            return false;
        }

        // 创建着色器资源视图
        if (desc.bindShaderResource) 
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            {
                srvDesc.Format = texDesc.Format;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = desc.mipLevels;
                srvDesc.Texture2D.MostDetailedMip = 0;
            }

            hr = device->GetDevice()->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv);
            if (FAILED(hr)) 
            {
                return false;
            }
        }

        // 创建渲染目标视图
        if (desc.bindRenderTarget) 
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            {
                rtvDesc.Format = texDesc.Format;
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
            }

            hr = device->GetDevice()->CreateRenderTargetView(m_texture.Get(), &rtvDesc, &m_rtv);
            if (FAILED(hr)) 
            {
                return false;
            }
        }

        return true;
    }

    bool Texture::CreateFromMemory(GraphicsDevice* device, const Desc& desc, const void* data) 
    {
        if (!Create(device, desc)) 
        {
            return false;
        }

        if (data) 
        {
            D3D11_BOX box = {};
            box.left = 0;
            box.right = desc.width;
            box.top = 0;
            box.bottom = desc.height;
            box.front = 0;
            box.back = 1;

            device->GetContext()->UpdateSubresource(m_texture.Get(), 0, &box, data, desc.width * 4, 0);
        }

        return true;
    }

    bool Texture::CreateFromD3DTexture(GraphicsDevice* device, ID3D11Texture2D* texture) 
    {
        m_texture = texture;

        D3D11_TEXTURE2D_DESC texDesc;
        texture->GetDesc(&texDesc);

        m_desc.width = texDesc.Width;
        m_desc.height = texDesc.Height;
        m_desc.mipLevels = texDesc.MipLevels;
        m_desc.arraySize = texDesc.ArraySize;
        m_desc.format = static_cast<TextureFormat>(texDesc.Format);
        m_desc.sampleCount = texDesc.SampleDesc.Count;
        m_desc.type = TextureType::Texture2D;

        // 根据绑定标志创建视图
        if (texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) 
        {
            device->GetDevice()->CreateShaderResourceView(texture, nullptr, &m_srv);
        }
        if (texDesc.BindFlags & D3D11_BIND_RENDER_TARGET) 
        {
            device->GetDevice()->CreateRenderTargetView(texture, nullptr, &m_rtv);
        }
        if (texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) 
        {
            device->GetDevice()->CreateUnorderedAccessView(texture, nullptr, &m_uav);
        }

        return true;
    }

    void Texture::UpdateData(GraphicsDevice* device, const void* data, size_t size, uint32_t mipLevel) 
    {
        D3D11_BOX box = {};
        {
            box.left = 0;
            box.top = 0;
            box.front = 0;
            box.right = m_desc.width >> mipLevel;
            box.bottom = m_desc.height >> mipLevel;
            box.back = 1;
        }
        device->GetContext()->UpdateSubresource(m_texture.Get(), mipLevel, &box, data,
            static_cast<UINT>(box.right * 4), 0);
    }

    D3D11_MAPPED_SUBRESOURCE Texture::Map(GraphicsDevice* device, uint32_t mipLevel, D3D11_MAP mapType) 
    {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = device->GetContext()->Map(m_texture.Get(), mipLevel, mapType, 0, &mappedResource);

        if (FAILED(hr)) 
        {
            return {};
        }

        return mappedResource;
    }

    void Texture::Unmap(GraphicsDevice* device, uint32_t mipLevel) 
    {
        device->GetContext()->Unmap(m_texture.Get(), mipLevel);
    }

    void Texture::GenerateMipmaps(GraphicsDevice* device) 
    {
        if (m_srv) 
        {
            device->GetContext()->GenerateMips(m_srv.Get());
        }
    }
}