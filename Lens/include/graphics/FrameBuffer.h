#pragma once

#include "RHI_dx11.h"
#include <memory>
#include <string>
#include <vector>

namespace lens::graphics
{
    class Framebuffer 
    {
    public:
        // 创建帧缓冲
        static std::shared_ptr<Framebuffer> Create(
            ID3D11Device* device,
            const std::vector<ID3D11RenderTargetView*>& rtvs,
            ID3D11DepthStencilView* dsv = nullptr);

        // 获取附件
        const std::vector<ID3D11RenderTargetView*>& GetRenderTargets() const { return m_rtvs; }
        ID3D11DepthStencilView* GetDepthStencil() const { return m_dsv; }

        // 获取尺寸等信息
        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }

    private:
        Framebuffer(const std::vector<ID3D11RenderTargetView*>& rtvs,
            ID3D11DepthStencilView* dsv,
            uint32_t width, uint32_t height);

        std::vector<ID3D11RenderTargetView*> m_rtvs;
        ID3D11DepthStencilView* m_dsv;
        uint32_t m_width;
        uint32_t m_height;
    };
}