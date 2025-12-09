#pragma once

#include "RHI_dx11.h"
#include <memory>
#include <string>
#include <wrl/client.h>

namespace lens::graphics
{
    class Texture
    {
    public:
        enum class Usage
        {
            Default,
            Dynamic,
            Staging,
            Immutable
        };

        static std::shared_ptr<Texture> Create2D(
            ID3D11Device* device,
            uint32_t width, uint32_t height,
            DXGI_FORMAT format,
            Usage usage,
            const void* initData = nullptr);

        static std::shared_ptr<Texture> CreateD3DTexture(ID3D11Texture2D* texture, bool takeOwnership = false);
        
        static std::shared_ptr<Texture> FromFile(ID3D11Device* device, const std::wstring& path);

        ID3D11Texture2D* GetTexture() const { return m_texture.Get(); }
        ID3D11ShaderResourceView* GetSRV() const { return m_srv; }
        ID3D11RenderTargetView* GetRTV() const { return m_rtv; }
        ID3D11DepthStencilView* GetDSV() const { return m_dsv; }

    private:
        Texture() = default;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

        ID3D11ShaderResourceView* m_srv;
        ID3D11RenderTargetView* m_rtv;
        ID3D11DepthStencilView* m_dsv;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
        Usage m_usage = Usage::Default;
        bool m_ownsTexture = false;
    };
}
