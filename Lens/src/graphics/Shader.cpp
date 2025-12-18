#include "LensPch.h"
#include "graphics/Shader.h"
#include <fstream>
#include "Log.h"

namespace lens::graphics 
{

    bool Shader::LoadVertexShader(GraphicsDevice* device, const std::string& filename, const char* entryPoint) 
    {
        ComPtr<ID3DBlob> blob;
        if (!CompileShader(filename, entryPoint, "vs_5_0", blob)) 
        {
            return false;
        }

        HRESULT hr = device->GetDevice()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_vertexShader);
        if (FAILED(hr)) 
        {
            return false;
        }

        // 保存字节码用于输入布局创建
        m_vertexShaderBytecode.assign(
            static_cast<char*>(blob->GetBufferPointer()),
            static_cast<char*>(blob->GetBufferPointer()) + blob->GetBufferSize()
        );

        return true;
    }

    bool Shader::LoadPixelShader(GraphicsDevice* device, const std::string& filename, const char* entryPoint) 
    {
        ComPtr<ID3DBlob> blob;
        if (!CompileShader(filename, entryPoint, "ps_5_0", blob)) 
        {
            return false;
        }

        HRESULT hr = device->GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_pixelShader);
        if (FAILED(hr)) 
        {
            return false;
        }

        return true;
    }

    bool Shader::LoadComputeShader(GraphicsDevice* device, const std::string& filename, const char* entryPoint) 
    {
        ComPtr<ID3DBlob> blob;
        if (!CompileShader(filename, entryPoint, "cs_5_0", blob)) 
        {
            return false;
        }

        HRESULT hr = device->GetDevice()->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_computeShader);
        if (FAILED(hr)) 
        {
            return false;
        }

        return true;
    }

    bool Shader::LoadVertexShaderFromBytecode(GraphicsDevice* device, const void* bytecode, size_t size) {
        HRESULT hr = device->GetDevice()->CreateVertexShader(bytecode, size, nullptr, &m_vertexShader);
        if (FAILED(hr)) 
        {
            return false;
        }

        // 保存字节码
        m_vertexShaderBytecode.assign(
            static_cast<const char*>(bytecode),
            static_cast<const char*>(bytecode) + size
        );

        return true;
    }

    bool Shader::LoadPixelShaderFromBytecode(GraphicsDevice* device, const void* bytecode, size_t size) 
    {
        HRESULT hr = device->GetDevice()->CreatePixelShader(bytecode, size, nullptr, &m_pixelShader);
        return SUCCEEDED(hr);
    }

    bool Shader::LoadComputeShaderFromBytecode(GraphicsDevice* device, const void* bytecode, size_t size) 
    {
        HRESULT hr = device->GetDevice()->CreateComputeShader(bytecode, size, nullptr, &m_computeShader);
        return SUCCEEDED(hr);
    }

    bool Shader::SetInputLayout(GraphicsDevice* device, const D3D11_INPUT_ELEMENT_DESC* layout, uint32_t layoutCount) 
    {
        if (m_vertexShaderBytecode.empty()) 
        {
            return false;
        }

        HRESULT hr = device->GetDevice()->CreateInputLayout(
            layout, layoutCount,
            m_vertexShaderBytecode.data(),
            m_vertexShaderBytecode.size(),
            &m_inputLayout
        );

        return SUCCEEDED(hr);
    }

    void Shader::SetToPipeline(GraphicsDevice* device) const 
    {
        auto context = device->GetContext();

        if (m_vertexShader) {
            context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
        }
        if (m_pixelShader) {
            context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
        }
        if (m_computeShader) {
            context->CSSetShader(m_computeShader.Get(), nullptr, 0);
        }
        if (m_inputLayout) {
            context->IASetInputLayout(m_inputLayout.Get());
        }
    }

    bool Shader::CompileShader(const std::string& filename, const char* entryPoint, const char* target, ComPtr<ID3DBlob>& blob) 
    {
        // 读取文件
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return false;
        }

        std::vector<char> source(file.tellg());
        file.seekg(0);
        file.read(source.data(), source.size());

        ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3DCompile(
            source.data(), source.size(),
            filename.c_str(), nullptr, nullptr,
            entryPoint, target,
            D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
            0, &blob, &errorBlob
        );

        if (FAILED(hr)) {
            if (errorBlob) {
                // 输出编译错误
                OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
            }
            return false;
        }

        return true;
    }

}