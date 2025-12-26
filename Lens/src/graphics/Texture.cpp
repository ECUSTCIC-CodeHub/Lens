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
        if (!texture)
        {
            LOG_ERROR("Null texture provided to CreateFromD3DTexture");
            return false;
        }

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

        // 如果原纹理没有着色器资源绑定标志，需要创建一个带该标志的副本
        if (!(texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
        {
            D3D11_TEXTURE2D_DESC newDesc = texDesc;
            newDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            newDesc.Usage = D3D11_USAGE_DEFAULT;
            newDesc.CPUAccessFlags = 0;
            newDesc.MiscFlags = 0;

            Microsoft::WRL::ComPtr<ID3D11Texture2D> newTexture;
            HRESULT hr = device->GetDevice()->CreateTexture2D(&newDesc, nullptr, &newTexture);
            if (FAILED(hr))
            {
                LOG_ERROR("Failed to create texture with SHADER_RESOURCE flag: 0x{:X}", hr);
                return false;
            }

            device->GetContext()->CopyResource(newTexture.Get(), texture);
            device->GetContext()->Flush();

            m_texture = newTexture;
        }

        // 创建着色器资源视图
        HRESULT hr = device->GetDevice()->CreateShaderResourceView(m_texture.Get(), nullptr, &m_srv);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create shader resource view: 0x{:X}", hr);
            return false;
        }

        // 根据绑定标志创建其他视图
        if (texDesc.BindFlags & D3D11_BIND_RENDER_TARGET)
        {
            device->GetDevice()->CreateRenderTargetView(m_texture.Get(), nullptr, &m_rtv);
        }
        if (texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
        {
            device->GetDevice()->CreateUnorderedAccessView(m_texture.Get(), nullptr, &m_uav);
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

    bool Texture::SaveToFile(GraphicsDevice* device, const char* filename)
    {
        if (!m_texture)
        {
            LOG_ERROR("No texture to save");
            return false;
        }

        // 创建一个可读写的staging纹理
        D3D11_TEXTURE2D_DESC stagingDesc = {};
        m_texture->GetDesc(&stagingDesc);
        stagingDesc.Usage = D3D11_USAGE_STAGING;
        stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingDesc.BindFlags = 0;
        stagingDesc.MiscFlags = 0;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
        HRESULT hr = device->GetDevice()->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to create staging texture: 0x{:X}", hr);
            return false;
        }

        // 复制纹理到staging纹理
        device->GetContext()->CopyResource(stagingTexture.Get(), m_texture.Get());

        // 映射staging纹理
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        hr = device->GetContext()->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed to map staging texture: 0x{:X}", hr);
            return false;
        }

        // 创建BMP文件
        int width = m_desc.width;
        int height = m_desc.height;
        int rowSize = width * 4; // BGRA8格式

        // 创建BMP文件头
        unsigned char fileHeader[14] = { 0 };
        unsigned char infoHeader[40] = { 0 };

        int fileSize = 14 + 40 + rowSize * height;
        fileHeader[0] = 'B';
        fileHeader[1] = 'M';
        *(int*)&fileHeader[2] = fileSize;
        *(int*)&fileHeader[10] = 54;

        infoHeader[0] = 40;
        *(int*)&infoHeader[4] = width;
        *(int*)&infoHeader[8] = height;
        infoHeader[12] = 1;
        infoHeader[14] = 32; // 32位BGRA

        // 写入文件
        FILE* f = nullptr;
        errno_t err = fopen_s(&f, filename, "wb");
        if (err != 0 || !f)
        {
            LOG_ERROR("Failed to create file: {}", filename);
            device->GetContext()->Unmap(stagingTexture.Get(), 0);
            return false;
        }

        fwrite(fileHeader, 1, 14, f);
        fwrite(infoHeader, 1, 40, f);

        // 写入像素数据（BMP是从下到上的）
        unsigned char* rowPtr = static_cast<unsigned char*>(mappedResource.pData);
        for (int y = height - 1; y >= 0; y--)
        {
            fwrite(rowPtr + y * mappedResource.RowPitch, 1, rowSize, f);
        }

        fclose(f);
        device->GetContext()->Unmap(stagingTexture.Get(), 0);

        LOG_INFO("Texture saved to {}", filename);
        return true;
    }
}