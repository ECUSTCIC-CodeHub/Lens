#pragma once

#include "RHI_dx11.h"
#include <memory>
#include <string>
#include <vector>

namespace lens::graphics
{
    // 着色器资源
    class Shader 
    {
    public:
        struct InputElement 
        {
            LPCSTR semanticName;
            UINT semanticIndex;
            DXGI_FORMAT format;
            UINT inputSlot;
            UINT alignedByteOffset;
        };

        static std::shared_ptr<Shader> Create(
            ID3D11Device* device,
            const std::wstring& vsPath,
            const std::wstring& psPath,
            const std::vector<InputElement>& inputLayout);

        ID3D11VertexShader* VS() const { return m_vs; }
        ID3D11PixelShader* PS() const { return m_ps; }
        ID3D11InputLayout* Layout() const { return m_inputLayout; }

    private:
        Shader(ID3D11VertexShader* vs, ID3D11PixelShader* ps, ID3D11InputLayout* layout);

        ID3D11VertexShader* m_vs;
        ID3D11PixelShader* m_ps;
        ID3D11InputLayout* m_inputLayout;
    };
}