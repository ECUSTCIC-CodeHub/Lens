# Graphics模块重新设计 - 代码实现

## 概述

本文档提供了重新设计的graphics模块的完整代码实现，具有以下特性：
- **通用性**：不专门绑定ImGui工作流
- **高性能**：低开销，最小化抽象层
- **可扩展**：易于添加新的渲染技术
- **WGC友好**：专为Windows Graphics Capture集成而设计

## 核心架构

新的graphics模块遵循以下原则：
1. **资源与用途分离**：资源不假设特定的使用场景
2. **现代管线抽象**：类似于现代图形API
3. **显式状态管理**：清晰、可预测的渲染行为
4. **ImGui作为应用层**：ImGui集成建立在通用接口之上

---

## 1. 核心设备接口

### GraphicsDevice.h

```cpp
#pragma once

#include <cstdint>
#include <span>
#include <memory>
#include <vector>

namespace lens::graphics {

// 前向声明
class Texture;
class Buffer;
class Shader;
class Pipeline;
class CommandList;
class TextureView;

enum class TextureFormat {
    RGBA8_UNorm,
    BGRA8_UNorm,
    R32_Float,
    RG32_Float,
    RGBA32_Float,
    R16_UInt,
    D24_UNorm_S8_UInt
};

enum class BufferUsage {
    Static,      // GPU只读，CPU一次性写入
    Dynamic,     // CPU读写，GPU只读
    Staging      // CPU读写，GPU访问禁用
};

enum class ShaderStage {
    Vertex,
    Pixel,
    Geometry,
    Compute,
    Hull,
    Domain
};

class GraphicsDevice {
public:
    struct Desc {
        void* windowHandle = nullptr;
        uint32_t width = 1920;
        uint32_t height = 1080;
        bool enableDebug = false;
    };

    static GraphicsDevice* Create(const Desc& desc);
    virtual ~GraphicsDevice() = default;

    // 帧控制
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present(bool vsync = true) = 0;
    virtual void Resize(uint32_t width, uint32_t height) = 0;

    // 资源创建
    virtual std::unique_ptr<Texture> CreateTexture1D(uint32_t width, TextureFormat format,
                                                    uint32_t arraySize = 1, uint32_t mipLevels = 1) = 0;
    virtual std::unique_ptr<Texture> CreateTexture2D(uint32_t width, uint32_t height, TextureFormat format,
                                                    uint32_t arraySize = 1, uint32_t mipLevels = 1,
                                                    uint32_t sampleCount = 1) = 0;
    virtual std::unique_ptr<Texture> CreateTexture3D(uint32_t width, uint32_t height, uint32_t depth,
                                                    TextureFormat format, uint32_t mipLevels = 1) = 0;

    virtual std::unique_ptr<Buffer> CreateBuffer(size_t size, BufferUsage usage,
                                                const void* initialData = nullptr) = 0;

    virtual std::unique_ptr<Shader> CreateShader() = 0;
    virtual std::unique_ptr<Pipeline> CreatePipeline() = 0;
    virtual std::unique_ptr<CommandList> CreateCommandList() = 0;

    // 状态管理
    virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
    virtual void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) = 0;

    // 查询设备能力
    virtual bool SupportsFeature(const char* feature) const = 0;
    virtual void* GetNativeDevice() const = 0;  // 需要时直接访问D3D11
};

} // namespace lens::graphics
```

---

## 2. 纹理资源系统

### Texture.h

```cpp
#pragma once

#include "GraphicsDevice.h"
#include <memory>

namespace lens::graphics {

class TextureView;

enum class TextureType {
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube
};

class Texture {
public:
    struct Desc {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t arraySize = 1;
        uint32_t mipLevels = 1;
        uint32_t sampleCount = 1;
        TextureFormat format = TextureFormat::RGBA8_UNorm;
        TextureType type = TextureType::Texture2D;
    };

    virtual ~Texture() = default;

    // 纹理信息
    virtual const Desc& GetDesc() const = 0;
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetDepth() const = 0;
    virtual TextureFormat GetFormat() const = 0;
    virtual TextureType GetType() const = 0;

    // 视图创建 - 这是资源与用途分离的关键
    virtual std::unique_ptr<TextureView> CreateView(TextureFormat format, uint32_t mipSlice = 0, uint32_t mipLevels = 1) = 0;
    virtual std::unique_ptr<TextureView> CreateArrayView(uint32_t firstArraySlice, uint32_t arraySize,
                                                         TextureFormat format, uint32_t mipSlice = 0, uint32_t mipLevels = 1) = 0;

    // 数据访问
    virtual void UpdateData(const void* data, size_t size, uint32_t mipLevel = 0) = 0;
    virtual void* Map(uint32_t mipLevel = 0) = 0;
    virtual void Unmap(uint32_t mipLevel = 0) = 0;

    // 生成mipmap
    virtual void GenerateMipmaps() = 0;

protected:
    Texture() = default;
};

enum class TextureViewType {
    ShaderResource,     // 用于着色器采样
    RenderTarget,       // 用于渲染目标输出
    UnorderedAccess,    // 用于计算着色器读写
    DepthStencil        // 用于深度/模板操作
};

class TextureView {
public:
    virtual ~TextureView() = default;

    virtual TextureViewType GetType() const = 0;
    virtual Texture* GetTexture() const = 0;
    virtual TextureFormat GetFormat() const = 0;

    // 获取原生视图，需要时直接API访问
    virtual void* GetNativeView() const = 0;

protected:
    TextureView() = default;
};

} // namespace lens::graphics
```

---

## 3. 缓冲区资源系统

### Buffer.h

```cpp
#pragma once

#include "GraphicsDevice.h"
#include <memory>

namespace lens::graphics {

enum class BufferType {
    Vertex,         // 顶点缓冲区
    Index,          // 索引缓冲区
    Constant,       // 常量/统一缓冲区
    Structured,     // 结构化缓冲区（用于计算着色器）
    Raw,            // 原始缓冲区（字节寻址）
    Append,         // 附加/消费缓冲区
    Counter         // 计数器缓冲区
};

enum class MapMode {
    Read,           // 只读访问
    Write,          // 只写访问
    ReadWrite,      // 读写访问
    WriteDiscard,   // 只写，丢弃先前内容
    WriteNoOverwrite // 只写，不覆盖现有数据
};

class Buffer {
public:
    struct Desc {
        size_t size = 0;
        BufferType type = BufferType::Vertex;
        BufferUsage usage = BufferUsage::Static;
        uint32_t structureStride = 0;  // 用于结构化缓冲区
        bool isUAV = false;            // 可用作UAV
    };

    virtual ~Buffer() = default;

    // 缓冲区信息
    virtual const Desc& GetDesc() const = 0;
    virtual size_t GetSize() const = 0;
    virtual BufferType GetType() const = 0;

    // 数据管理
    virtual void UpdateData(const void* data, size_t size, size_t offset = 0) = 0;
    virtual void* Map(MapMode mode) = 0;
    virtual void Unmap() = 0;

    // 获取原生缓冲区，直接API访问
    virtual void* GetNativeBuffer() const = 0;

    // 特定缓冲区类型的辅助方法
    template<typename T>
    void UpdateStruct(const T& data) {
        UpdateData(&data, sizeof(T));
    }

    template<typename T>
    T* MapAs() {
        return static_cast<T*>(Map(MapMode::Write));
    }

protected:
    Buffer() = default;
};

} // namespace lens::graphics
```

---

## 4. 着色器系统

### Shader.h

```cpp
#pragma once

#include "GraphicsDevice.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace lens::graphics {

enum class ShaderLanguage {
    HLSL,
    GLSL
};

class ShaderReflection {
public:
    struct ResourceBinding {
        std::string name;
        uint32_t slot;
        ShaderStage stage;
        enum class Type {
            ConstantBuffer,
            Texture,
            Sampler,
            UnorderedAccess
        } type;
    };

    virtual ~ShaderReflection() = default;
    virtual const std::vector<ResourceBinding>& GetResourceBindings() const = 0;
    virtual uint32_t GetInputParameterCount() const = 0;
    virtual bool HasStage(ShaderStage stage) const = 0;
};

class Shader {
public:
    virtual ~Shader() = default;

    // 从不同源加载着色器阶段
    virtual bool LoadFromFile(ShaderStage stage, const std::string& filename,
                              const char* entryPoint = "main", ShaderLanguage lang = ShaderLanguage::HLSL) = 0;
    virtual bool LoadFromMemory(ShaderStage stage, const void* bytecode, size_t size) = 0;
    virtual bool LoadFromSource(ShaderStage stage, const std::string& source,
                                const char* entryPoint = "main", ShaderLanguage lang = ShaderLanguage::HLSL) = 0;

    // 着色器信息
    virtual bool HasStage(ShaderStage stage) const = 0;
    virtual ShaderReflection* GetReflection() const = 0;

    // 资源绑定辅助函数
    virtual bool SetConstantBuffer(const std::string& name, Buffer* buffer) = 0;
    virtual bool SetTexture(const std::string& name, TextureView* view) = 0;
    virtual bool SetSampler(const std::string& name, class Sampler* sampler) = 0;

    // 基于插槽的绑定（用于性能）
    virtual void SetConstantBuffer(uint32_t slot, Buffer* buffer, ShaderStage stage = ShaderStage::Pixel) = 0;
    virtual void SetTexture(uint32_t slot, TextureView* view, ShaderStage stage = ShaderStage::Pixel) = 0;
    virtual void SetSampler(uint32_t slot, class Sampler* sampler, ShaderStage stage = ShaderStage::Pixel) = 0;

    // 获取原生着色器句柄
    virtual void* GetNativeVertexShader() const = 0;
    virtual void* GetNativePixelShader() const = 0;
    virtual void* GetNativeComputeShader() const = 0;
    virtual void* GetNativeGeometryShader() const = 0;

protected:
    Shader() = default;
};

} // namespace lens::graphics
```

---

## 5. 管线系统

### Pipeline.h

```cpp
#pragma once

#include "GraphicsDevice.h"
#include "Shader.h"
#include "Buffer.h"
#include "Texture.h"
#include <memory>
#include <vector>

namespace lens::graphics {

enum class PrimitiveTopology {
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip
};

enum class FillMode {
    Solid,
    Wireframe
};

enum class CullMode {
    None,
    Front,
    Back
};

enum class ComparisonFunc {
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always
};

enum class BlendOp {
    Add,
    Subtract,
    RevSubtract,
    Min,
    Max
};

enum class BlendFactor {
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DstAlpha,
    InvDstAlpha,
    DstColor,
    InvDstColor,
    SrcAlphaSat,
    BlendFactor,
    InvBlendFactor,
    Src1Color,
    InvSrc1Color,
    Src1Alpha,
    InvSrc1Alpha
};

class BlendState {
public:
    struct Desc {
        bool blendEnable = false;
        BlendOp rgbBlendOp = BlendOp::Add;
        BlendOp alphaBlendOp = BlendOp::Add;
        BlendFactor srcBlend = BlendFactor::One;
        BlendFactor dstBlend = BlendFactor::Zero;
        BlendFactor srcBlendAlpha = BlendFactor::One;
        BlendFactor dstBlendAlpha = BlendFactor::Zero;
        uint8_t writeMask = 0xF;  // RGBA
    };

    static BlendState* Create(const Desc& desc);
    virtual ~BlendState() = default;
    virtual void* GetNativeState() const = 0;
};

class RasterizerState {
public:
    struct Desc {
        FillMode fillMode = FillMode::Solid;
        CullMode cullMode = CullMode::Back;
        bool frontCounterClockwise = false;
        int depthBias = 0;
        float depthBiasClamp = 0.0f;
        float slopeScaledDepthBias = 0.0f;
        bool depthClipEnable = true;
        bool scissorEnable = false;
        bool multisampleEnable = false;
        bool antialiasedLineEnable = false;
    };

    static RasterizerState* Create(const Desc& desc);
    virtual ~RasterizerState() = default;
    virtual void* GetNativeState() const = 0;
};

class DepthStencilState {
public:
    struct Desc {
        bool depthEnable = true;
        bool depthWriteEnable = true;
        ComparisonFunc depthFunc = ComparisonFunc::Less;
        bool stencilEnable = false;
        uint8_t stencilReadMask = 0xFF;
        uint8_t stencilWriteMask = 0xFF;

        // 正面操作
        ComparisonFunc frontStencilFunc = ComparisonFunc::Always;
        BlendOp frontStencilFailOp = BlendOp::Add;
        BlendOp frontStencilDepthFailOp = BlendOp::Add;
        BlendOp frontStencilPassOp = BlendOp::Add;

        // 背面操作
        ComparisonFunc backStencilFunc = ComparisonFunc::Always;
        BlendOp backStencilFailOp = BlendOp::Add;
        BlendOp backStencilDepthFailOp = BlendOp::Add;
        BlendOp backStencilPassOp = BlendOp::Add;
    };

    static DepthStencilState* Create(const Desc& desc);
    virtual ~DepthStencilState() = default;
    virtual void* GetNativeState() const = 0;
};

class Pipeline {
public:
    struct Desc {
        Shader* shader = nullptr;

        // 输入布局（用于顶点处理）
        struct InputElement {
            std::string semanticName;
            uint32_t semanticIndex;
            TextureFormat format;
            uint32_t inputSlot;
            uint32_t alignedByteOffset;
            bool perInstance;
            uint32_t instanceDataStepRate;
        };
        std::vector<InputElement> inputElements;

        // 渲染目标
        std::vector<TextureFormat> renderTargetFormats;
        TextureFormat depthStencilFormat = TextureFormat::D24_UNorm_S8_UInt;

        // 状态
        BlendState* blendState = nullptr;
        RasterizerState* rasterizerState = nullptr;
        DepthStencilState* depthStencilState = nullptr;

        // 图元拓扑
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        // MSAA采样数
        uint32_t sampleCount = 1;

        // 管线类型
        bool isComputePipeline = false;
    };

    virtual ~Pipeline() = default;

    // 资源绑定
    virtual void SetVertexBuffers(std::span<Buffer*> buffers, std::span<uint32_t> strides, std::span<uint32_t> offsets) = 0;
    virtual void SetIndexBuffer(Buffer* buffer, bool use32Bit = true) = 0;
    virtual void SetConstantBuffer(uint32_t slot, Buffer* buffer) = 0;
    virtual void SetTexture(uint32_t slot, TextureView* view) = 0;
    virtual void SetSampler(uint32_t slot, class Sampler* sampler) = 0;
    virtual void SetRenderTargets(std::span<TextureView*> renderTargets, TextureView* depthStencil = nullptr) = 0;

    // 视口和裁剪矩形
    virtual void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
    virtual void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) = 0;

protected:
    Pipeline() = default;
};

} // namespace lens::graphics
```

---

## 6. 命令列表系统

### CommandList.h

```cpp
#pragma once

#include "Pipeline.h"
#include <memory>

namespace lens::graphics {

class CommandList {
public:
    virtual ~CommandList() = default;

    // 命令录制控制
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void Reset() = 0;
    virtual void Execute() = 0;  // 提交到GPU队列

    // 管线和资源绑定
    virtual void SetPipeline(Pipeline* pipeline) = 0;
    virtual void SetVertexBuffers(std::span<Buffer*> buffers, std::span<uint32_t> strides, std::span<uint32_t> offsets) = 0;
    virtual void SetIndexBuffer(Buffer* buffer, bool use32Bit = true) = 0;
    virtual void SetConstantBuffer(uint32_t slot, Buffer* buffer) = 0;
    virtual void SetTexture(uint32_t slot, TextureView* view) = 0;
    virtual void SetSampler(uint32_t slot, class Sampler* sampler) = 0;
    virtual void SetRenderTargets(std::span<TextureView*> renderTargets, TextureView* depthStencil = nullptr) = 0;

    // 绘制命令
    virtual void Draw(uint32_t vertexCount, uint32_t startVertex = 0) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndex = 0, int32_t baseVertex = 0) = 0;
    virtual void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount,
                               uint32_t startVertex = 0, uint32_t startInstance = 0) = 0;
    virtual void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount,
                                      uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t startInstance = 0) = 0;

    // 计算命令
    virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;
    virtual void DispatchIndirect(Buffer* argumentBuffer, size_t offset) = 0;

    // 资源操作
    virtual void CopyTexture(Texture* dst, Texture* src, uint32_t dstMip = 0, uint32_t srcMip = 0) = 0;
    virtual void CopyBuffer(Buffer* dst, Buffer* src, size_t size, size_t dstOffset = 0, size_t srcOffset = 0) = 0;
    virtual void ClearRenderTarget(TextureView* rtv, const float color[4]) = 0;
    virtual void ClearDepthStencil(TextureView* dsv, float depth, uint8_t stencil) = 0;
    virtual void ClearUnorderedAccessView(TextureView* uav, const void* clearValue, size_t clearValueSize) = 0;

    // 屏障和同步
    virtual void ResourceBarrier(Texture* texture, enum class ResourceState before, enum class ResourceState after) = 0;
    virtual void ResourceBarrier(Buffer* buffer, enum class ResourceState before, enum class ResourceState after) = 0;
    virtual void UAVBarrier() = 0;

    // 调试
    virtual void BeginEvent(const char* name) = 0;
    virtual void EndEvent() = 0;
    virtual void SetMarker(const char* name) = 0;

protected:
    CommandList() = default;
};

} // namespace lens::graphics
```

---

## 7. WGC集成层

### WGCTextureProcessor.h

```cpp
#pragma once

#include "Texture.h"
#include <windows.graphics.capture.h>
#include <winrt/windows.graphics.directx.h>
#include <winrt/windows.graphics.directx.direct3d11.h>
#include <memory>

namespace lens::wgc {

class WGCTextureProcessor {
public:
    WGCTextureProcessor(graphics::GraphicsDevice* device);
    ~WGCTextureProcessor();

    // 将WGC纹理转换为我们的通用Texture
    std::unique_ptr<graphics::Texture> ProcessWGCTexture(
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface wgcSurface
    );

    // 便捷方法，为处理管线创建输入纹理
    std::unique_ptr<graphics::Texture> CreateInputTextureFromWGC(
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface wgcSurface
    );

    // 多个WGC帧的批处理
    struct ProcessedFrame {
        std::unique_ptr<graphics::Texture> texture;
        uint64_t frameNumber;
        bool isNewFrame;
    };

    ProcessedFrame ProcessNextFrame(
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface wgcSurface
    );

private:
    graphics::GraphicsDevice* m_device;

    // 内部转换管线
    std::unique_ptr<graphics::Texture> m_intermediateTexture;
    std::unique_ptr<graphics::Pipeline> m_conversionPipeline;

    // 辅助方法
    graphics::TextureFormat ConvertWGCFormat(winrt::Windows::Graphics::DirectX::DirectXFormat format);
    bool NeedFormatConversion(graphics::TextureFormat srcFormat, graphics::TextureFormat dstFormat);
    void PerformFormatConversion(graphics::Texture* src, graphics::Texture* dst);
};

} // namespace lens::wgc
```

---

## 8. ImGui集成（应用层）

### ImGuiRenderer.h（位于gui/目录下）

```cpp
#pragma once

#include "../include/graphics/GraphicsDevice.h"
#include "../include/graphics/Texture.h"
#include "../include/graphics/Buffer.h"
#include "../include/graphics/Pipeline.h"
#include "../include/graphics/CommandList.h"
#include <memory>

// 前向声明ImGui类型
struct ImDrawData;
struct ImVec2;

namespace lens::gui {

class ImGuiRenderer {
public:
    ImGuiRenderer(graphics::GraphicsDevice* device);
    ~ImGuiRenderer();

    // 使用我们的图形后端初始化ImGui
    bool Initialize();

    // 帧管理
    void BeginFrame();
    void EndFrame();
    void Render();

    // 资源转换工具
    void* ConvertTextureForImGui(graphics::Texture* texture);
    graphics::Texture* CreateImGuiTexture(int width, int height, const void* data = nullptr);
    void UpdateImGuiTexture(graphics::Texture* texture, const void* data);

    // DPI和缩放支持
    void SetDisplayScale(float scaleX, float scaleY);
    ImVec2 GetDisplaySize() const;

    // 字体处理
    bool AddFontFromFile(const char* filename, float sizePixels);
    bool AddFontFromMemory(const void* data, size_t dataSize, float sizePixels);

private:
    graphics::GraphicsDevice* m_device;

    // ImGui专用图形资源
    std::unique_ptr<graphics::Pipeline> m_pipeline;
    std::unique_ptr<graphics::Buffer> m_vertexBuffer;
    std::unique_ptr<graphics::Buffer> m_indexBuffer;
    std::unique_ptr<graphics::Texture> m_fontTexture;
    std::unique_ptr<graphics::TextureView> m_fontTextureView;

    // 渲染用命令列表
    std::unique_ptr<graphics::CommandList> m_commandList;

    // 状态
    bool m_initialized = false;
    float m_displayScaleX = 1.0f;
    float m_displayScaleY = 1.0f;

    // 内部方法
    void CreateGraphicsResources();
    void SetupRenderState();
    void RenderDrawLists(ImDrawData* drawData);
    void SetupImGuiStyle();
};

} // namespace lens::gui
```

---

## 9. 使用示例

### 基础图像处理管线

```cpp
#include "GraphicsDevice.h"
#include "Texture.h"
#include "Pipeline.h"
#include "CommandList.h"
#include "WGCTextureProcessor.h"

class ImageProcessingApp {
private:
    std::unique_ptr<graphics::GraphicsDevice> m_device;
    std::unique_ptr<wgc::WGCTextureProcessor> m_wgcProcessor;
    std::unique_ptr<graphics::Pipeline> m_processingPipeline;
    std::unique_ptr<graphics::CommandList> m_commandList;
    std::unique_ptr<graphics::Texture> m_outputTexture;
    std::unique_ptr<graphics::TextureView> m_outputRTV;

public:
    bool Initialize() {
        // 创建设备
        graphics::GraphicsDevice::Desc deviceDesc{};
        deviceDesc.windowHandle = GetHwnd();  // 你的窗口句柄
        deviceDesc.width = 1920;
        deviceDesc.height = 1080;
        m_device = graphics::GraphicsDevice::Create(deviceDesc);

        // 初始化WGC处理器
        m_wgcProcessor = std::make_unique<wgc::WGCTextureProcessor>(m_device.get());

        // 为处理结果创建输出纹理
        m_outputTexture = m_device->CreateTexture2D(1920, 1080, graphics::TextureFormat::RGBA8_UNorm);
        m_outputRTV = m_outputTexture->CreateView(graphics::TextureFormat::RGBA8_UNorm);

        // 创建处理管线
        CreateProcessingPipeline();

        // 创建命令列表
        m_commandList = m_device->CreateCommandList();

        return true;
    }

    void ProcessWGCFrame(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface wgcSurface) {
        // 将WGC纹理转换为我们的格式
        auto inputTexture = m_wgcProcessor->ProcessWGCTexture(wgcSurface);
        auto inputSRV = inputTexture->CreateView(graphics::TextureFormat::RGBA8_UNorm);

        // 录制渲染命令
        m_commandList->Begin();
        m_commandList->SetPipeline(m_processingPipeline.get());
        m_commandList->SetRenderTargets({m_outputRTV.get()}, nullptr);
        m_commandList->SetTexture(0, inputSRV.get());

        // 绘制全屏三角形（图像处理的常用技术）
        m_commandList->Draw(3);

        m_commandList->End();
        m_commandList->Execute();
    }

    graphics::Texture* GetProcessedTexture() {
        return m_outputTexture.get();
    }

private:
    void CreateProcessingPipeline() {
        // 加载处理着色器
        auto shader = m_device->CreateShader();
        shader->LoadFromFile(graphics::ShaderStage::Vertex, "shaders/fullscreen_vs.hlsl");
        shader->LoadFromFile(graphics::ShaderStage::Pixel, "shaders/image_process_ps.hlsl");

        // 创建管线状态
        graphics::Pipeline::Desc pipelineDesc{};
        pipelineDesc.shader = shader.get();
        pipelineDesc.renderTargetFormats = {graphics::TextureFormat::RGBA8_UNorm};
        pipelineDesc.topology = graphics::PrimitiveTopology::TriangleList;

        m_processingPipeline = m_device->CreatePipeline(pipelineDesc);
    }
};
```

### ImGui集成示例

```cpp
#include "gui/ImGuiRenderer.h"
#include "WGCTextureProcessor.h"

class ApplicationUI {
private:
    std::unique_ptr<graphics::GraphicsDevice> m_device;
    std::unique_ptr<wgc::WGCTextureProcessor> m_wgcProcessor;
    std::unique_ptr<gui::ImGuiRenderer> m_imguiRenderer;
    graphics::Texture* m_currentFrameTexture = nullptr;

public:
    bool Initialize() {
        // ... 如上所述的设备初始化 ...

        // 初始化ImGui渲染器
        m_imguiRenderer = std::make_unique<gui::ImGuiRenderer>(m_device.get());
        m_imguiRenderer->Initialize();

        return true;
    }

    void RenderFrame() {
        // 处理WGC帧
        auto wgcSurface = CaptureWindow(); // 你的WGC捕获逻辑
        auto texture = m_wgcProcessor->ProcessWGCTexture(wgcSurface);
        m_currentFrameTexture = texture.get();

        // 开始ImGui帧
        m_imguiRenderer->BeginFrame();

        // 创建UI
        RenderMainWindow();

        // 结束ImGui帧并渲染
        m_imguiRenderer->EndFrame();
        m_device->BeginFrame();
        m_imguiRenderer->Render();
        m_device->EndFrame();
        m_device->Present(true);
    }

private:
    void RenderMainWindow() {
        ImGui::Begin("窗口捕获");

        if (m_currentFrameTexture) {
            // 将我们的通用纹理转换为ImGui格式
            void* imguiTexId = m_imguiRenderer->ConvertTextureForImGui(m_currentFrameTexture);

            // 显示纹理
            ImGui::Image(imguiTexId, ImVec2(800, 600));
        }

        ImGui::End();
    }
};
```

### 高级多通道处理

```cpp
class MultiPassProcessor {
private:
    struct ProcessingPass {
        std::unique_ptr<graphics::Pipeline> pipeline;
        std::unique_ptr<graphics::Texture> outputTexture;
        std::unique_ptr<graphics::TextureView> outputRTV;
        std::unique_ptr<graphics::TextureView> outputSRV;
    };

    std::vector<ProcessingPass> m_passes;
    std::unique_ptr<graphics::CommandList> m_commandList;

public:
    void AddProcessingPass(const std::string& pixelShader) {
        ProcessingPass pass;

        // 创建输出纹理
        pass.outputTexture = m_device->CreateTexture2D(1920, 1080, graphics::TextureFormat::RGBA8_UNorm);
        pass.outputRTV = pass.outputTexture->CreateView(graphics::TextureFormat::RGBA8_UNorm);
        pass.outputSRV = pass.outputTexture->CreateView(graphics::TextureFormat::RGBA8_UNorm);

        // 为此通道创建管线
        auto shader = m_device->CreateShader();
        shader->LoadFromFile(graphics::ShaderStage::Vertex, "shaders/fullscreen_vs.hlsl");
        shader->LoadFromFile(graphics::ShaderStage::Pixel, pixelShader.c_str());

        graphics::Pipeline::Desc pipelineDesc{};
        pipelineDesc.shader = shader.get();
        pipelineDesc.renderTargetFormats = {graphics::TextureFormat::RGBA8_UNorm};
        pipelineDesc.topology = graphics::PrimitiveTopology::TriangleList;
        pass.pipeline = m_device->CreatePipeline(pipelineDesc);

        m_passes.push_back(std::move(pass));
    }

    graphics::Texture* ProcessFrame(graphics::Texture* inputTexture) {
        auto inputSRV = inputTexture->CreateView(graphics::TextureFormat::RGBA8_UNorm);

        m_commandList->Begin();

        // 通过所有通道处理
        for (size_t i = 0; i < m_passes.size(); ++i) {
            auto& pass = m_passes[i];

            m_commandList->SetPipeline(pass.pipeline.get());
            m_commandList->SetRenderTargets({pass.outputRTV.get()}, nullptr);
            m_commandList->SetTexture(0, inputSRV.get());

            m_commandList->Draw(3);

            // 此通道的输出成为下一通道的输入
            if (i < m_passes.size() - 1) {
                inputSRV = pass.outputSRV.get();
            }
        }

        m_commandList->End();
        m_commandList->Execute();

        // 返回最终输出
        return m_passes.back().outputTexture.get();
    }
};
```

---

## 10. 与现有代码集成

### 更新RHI_dx11.h

```cpp
#pragma once

#include "GraphicsDevice.h"
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

namespace lens::graphics {

class RHI_dx11 : public GraphicsDevice {
public:
    static RHI_dx11* Create(const Desc& desc);
    virtual ~RHI_dx11();

    // GraphicsDevice接口实现
    void BeginFrame() override;
    void EndFrame() override;
    void Present(bool vsync) override;
    void Resize(uint32_t width, uint32_t height) override;

    // 资源创建
    std::unique_ptr<Texture> CreateTexture2D(uint32_t width, uint32_t height, TextureFormat format,
                                            uint32_t arraySize = 1, uint32_t mipLevels = 1,
                                            uint32_t sampleCount = 1) override;
    std::unique_ptr<Buffer> CreateBuffer(size_t size, BufferUsage usage, const void* initialData = nullptr) override;
    std::unique_ptr<Shader> CreateShader() override;
    std::unique_ptr<Pipeline> CreatePipeline() override;
    std::unique_ptr<CommandList> CreateCommandList() override;

    // D3D11专用访问
    ID3D11Device* GetD3DDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetD3DContext() const { return m_context.Get(); }
    IDXGISwapChain* GetSwapChain() const { return m_swapChain.Get(); }

private:
    RHI_dx11() = default;
    void Initialize(const Desc& desc);

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_defaultRTV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_defaultDSV;
};

} // namespace lens::graphics
```

### 更新Application.cpp

```cpp
#include "Application.h"
#include "graphics/GraphicsDevice.h"
#include "gui/ImGuiRenderer.h"
#include "gui/UIManager.h"

namespace lens {

class Application {
private:
    std::unique_ptr<graphics::GraphicsDevice> m_graphicsDevice;
    std::unique_ptr<gui::ImGuiRenderer> m_imguiRenderer;
    std::unique_ptr<gui::UIManager> m_uiManager;

public:
    bool Initialize() {
        // 初始化图形设备
        graphics::GraphicsDevice::Desc deviceDesc{};
        deviceDesc.windowHandle = m_hwnd;
        deviceDesc.width = m_width;
        deviceDesc.height = m_height;
        m_graphicsDevice = graphics::GraphicsDevice::Create(deviceDesc);

        // 初始化ImGui渲染器
        m_imguiRenderer = std::make_unique<gui::ImGuiRenderer>(m_graphicsDevice.get());
        m_imguiRenderer->Initialize();

        // 初始化UI管理器（你现有的系统）
        m_uiManager = std::make_unique<gui::UIManager>();
        m_uiManager->Initialize();

        return true;
    }

    void RunFrame() {
        // 处理消息
        // ...

        // 开始帧
        m_graphicsDevice->BeginFrame();
        m_imguiRenderer->BeginFrame();

        // 更新并渲染UI面板
        m_uiManager->Update();
        m_uiManager->Render();

        // 结束帧
        m_imguiRenderer->EndFrame();
        m_imguiRenderer->Render();
        m_graphicsDevice->EndFrame();
        m_graphicsDevice->Present(true);
    }
};

} // namespace lens
```

---

## 11. 着色器示例

### 全屏顶点着色器（fullscreen_vs.hlsl）

```hlsl
struct VS_INPUT {
    uint vertexID : SV_VertexID;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output;

    // 使用顶点ID生成全屏三角形
    // 此技术避免了需要顶点缓冲区
    float2 positions[3] = {
        float2(-1.0, -1.0),
        float2(3.0, -1.0),
        float2(-1.0, 3.0)
    };

    float2 texcoords[3] = {
        float2(0.0, 1.0),
        float2(2.0, 1.0),
        float2(0.0, -1.0)
    };

    output.position = float4(positions[input.vertexID], 0.0, 1.0);
    output.texcoord = texcoords[input.vertexID];

    return output;
}
```

### 基础图像处理像素着色器（image_process_ps.hlsl）

```hlsl
Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ProcessParams : register(b0) {
    float brightness;
    float contrast;
    float saturation;
    float padding;
};

float3 AdjustBrightness(float3 color, float brightness) {
    return color + brightness;
}

float3 AdjustContrast(float3 color, float contrast) {
    return (color - 0.5) * contrast + 0.5;
}

float3 AdjustSaturation(float3 color, float saturation) {
    float gray = dot(color, float3(0.299, 0.587, 0.114));
    return lerp(gray, color, saturation);
}

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0) : SV_TARGET {
    float4 color = inputTexture.Sample(samplerState, texcoord);

    // 应用图像处理
    color.rgb = AdjustBrightness(color.rgb, brightness);
    color.rgb = AdjustContrast(color.rgb, contrast);
    color.rgb = AdjustSaturation(color.rgb, saturation);

    return color;
}
```

### 高级处理像素着色器（blur_ps.hlsl）

```hlsl
Texture2D inputTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer BlurParams : register(b0) {
    float2 texelSize;
    float blurStrength;
    int blurRadius;
};

float4 GaussianBlur(float2 texcoord) {
    float4 color = float4(0, 0, 0, 0);
    float totalWeight = 0.0;

    for (int x = -blurRadius; x <= blurRadius; x++) {
        for (int y = -blurRadius; y <= blurRadius; y++) {
            float2 offset = float2(x, y) * texelSize * blurStrength;
            float weight = exp(-(x*x + y*y) / (2.0 * blurRadius * blurRadius));

            color += inputTexture.Sample(samplerState, texcoord + offset) * weight;
            totalWeight += weight;
        }
    }

    return color / totalWeight;
}

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0) : SV_TARGET {
    return GaussianBlur(texcoord);
}
```

---

## 12. 迁移指南

### 从旧系统迁移到新系统

1. **将直接的D3D11调用替换为GraphicsDevice接口**
   ```cpp
   // 旧方式
   deviceContext->Draw(vertexCount, startVertex);

   // 新方式
   commandList->Draw(vertexCount, startVertex);
   ```

2. **更新纹理创建**
   ```cpp
   // 旧方式
   ID3D11Texture2D* texture = nullptr;
   device->CreateTexture2D(&desc, nullptr, &texture);

   // 新方式
   auto texture = graphicsDevice->CreateTexture2D(width, height, format);
   ```

3. **使用基于视图的资源绑定**
   ```cpp
   // 旧方式
   deviceContext->PSSetShaderResources(0, 1, &srv);

   // 新方式
   auto textureView = texture->CreateView(format);
   pipeline->SetTexture(0, textureView.get());
   ```

4. **更新ImGui集成**
   ```cpp
   // 旧方式
   ImGui_ImplDX11_NewFrame();

   // 新方式
   imguiRenderer->BeginFrame();
   ```

### 性能考虑

1. **最小化视图创建**：一次性创建视图并重复使用
2. **批处理绘制调用**：使用CommandList批量操作
3. **资源池化**：考虑池化频繁创建的纹理/缓冲区
4. **异步操作**：可能时使用CommandList进行后台处理

### 测试新系统

1. **单元测试**：创建简单的纹理创建/访问测试
2. **集成测试**：测试WGC -> 处理 -> ImGui管线
3. **性能测试**：测量与直接D3D11调用的开销对比

---

## 结论

这个重新设计的graphics模块提供：

1. **现代架构**：类似于现代图形API
2. **灵活性**：资源可以通过视图多种方式使用
3. **性能**：在提供高级抽象的同时保持低开销
4. **可维护性**：清晰的关注点和职责分离
5. **可扩展性**：易于添加新功能而不破坏现有代码
6. **ImGui兼容性**：ImGui成为应用层，而不是核心要求

该系统设计得足够通用以处理任何图形任务，同时专门为你的WGC + 着色器处理 + ImGui工作流优化。