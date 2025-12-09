#include "LensPch.h"
#include "graphics/Texture.h"
#include "RHI_dx11.h"
#include "Log.h"

namespace lens::graphics
{
    std::shared_ptr<Texture> Texture::Create2D(
        ID3D11Device* device,
        uint32_t width, uint32_t height,
        DXGI_FORMAT format,
        Usage usage,
        const void* initData)
    {
        
        if(!device || width == 0 || height == 0 || format == DXGI_FORMAT_UNKNOWN)
        {
            LOG_ERROR("Invalid parameters for Texture::Create2D");
            return nullptr;
        }

        auto texture = std::shared_ptr<Texture>();
        {
            texture->m_width = width;
            texture->m_height = height;
            texture->m_format = format;
            texture->m_usage = usage;
            texture->m_ownsTexture = true;
        }

        D3D11_TEXTURE2D_DESC desc = {};
        {
            desc.Width = width;
            desc.Height = height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = format;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
        }

        switch (usage)
        {
        case Usage::Dynamic:
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            break;
        case Usage::Staging:
            desc.Usage = D3D11_USAGE_STAGING;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        default:
            desc.Usage = D3D11_USAGE_DEFAULT;
            break;
        }


        return nullptr;
    }
}