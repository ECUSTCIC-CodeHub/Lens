#pragma once

#include "GraphicsDevice.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <string>
#include <vector>

namespace lens::graphics 
{

    using Microsoft::WRL::ComPtr;

    class Shader 
    {
    public:
        Shader() = default;
        ~Shader() = default;

        // 从文件编译加载
        bool LoadVertexShader(GraphicsDevice* device, const std::string& filename, const char* entryPoint = "main");
        bool LoadPixelShader(GraphicsDevice* device, const std::string& filename, const char* entryPoint = "main");
        bool LoadComputeShader(GraphicsDevice* device, const std::string& filename, const char* entryPoint = "main");

        // 从已编译字节码加载
        bool LoadVertexShaderFromBytecode(GraphicsDevice* device, const void* bytecode, size_t size);
        bool LoadPixelShaderFromBytecode(GraphicsDevice* device, const void* bytecode, size_t size);
        bool LoadComputeShaderFromBytecode(GraphicsDevice* device, const void* bytecode, size_t size);

        // 直接访问
        ID3D11VertexShader* GetVertexShader() const { return m_vertexShader.Get(); }
        ID3D11PixelShader* GetPixelShader() const { return m_pixelShader.Get(); }
        ID3D11ComputeShader* GetComputeShader() const { return m_computeShader.Get(); }
        ID3D11InputLayout* GetInputLayout() const { return m_inputLayout.Get(); }

        // 状态检查
        bool HasVertexShader() const { return m_vertexShader != nullptr; }
        bool HasPixelShader() const { return m_pixelShader != nullptr; }
        bool HasComputeShader() const { return m_computeShader != nullptr; }

        // 输入布局设置
        bool SetInputLayout(GraphicsDevice* device, const D3D11_INPUT_ELEMENT_DESC* layout, uint32_t layoutCount);

        // 便捷方法：设置着色器到管线
        void SetToPipeline(GraphicsDevice* device) const;

    private:
        ComPtr<ID3D11VertexShader> m_vertexShader;
        ComPtr<ID3D11PixelShader> m_pixelShader;
        ComPtr<ID3D11ComputeShader> m_computeShader;
        ComPtr<ID3D11InputLayout> m_inputLayout;

        std::vector<char> m_vertexShaderBytecode;

        bool CompileShader(const std::string& filename, const char* entryPoint, const char* target, ComPtr<ID3DBlob>& blob);
    };

}