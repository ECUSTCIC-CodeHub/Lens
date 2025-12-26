#pragma once

#include "graphics/GraphicsDevice.h"
#include "graphics/Texture.h"
#include <windows.graphics.capture.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.System.h>

namespace lens::capturer
{
    class WGCCapturer
    {
    public:
        struct CaptureSource
        {
            HWND windowHandle = nullptr;
            std::wstring windowTitle;
            RECT windowRect;
        };

        struct CaptureDesc
        {
            uint32_t frameRate = 30;
            lens::graphics::TextureFormat format = lens::graphics::TextureFormat::BGRA8_UNorm;
            bool captureCursor = true;
            bool captureBorder = true;
        };

        WGCCapturer(lens::graphics::GraphicsDevice* device);
        ~WGCCapturer();
        // 初始化和清理
        bool Initialize(const CaptureDesc& desc);
        void Shutdown();

        // 捕获控制
        bool StartCapture(HWND window);
        void StopCapture();
        bool IsCapturing() const { return m_isCapturing; }

        // 帧获取
        std::shared_ptr<lens::graphics::Texture> GetLatestFrame();
        bool HasNewFrame() const { return m_frameAvailable; }

        // 源枚举
        static std::vector<CaptureSource> EnumerateWindows();

    private:
        lens::graphics::GraphicsDevice* m_device;
        CaptureDesc m_desc;

        // WinRT WGC 对象
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_captureItem{ nullptr };
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
        winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ nullptr };

        // 帧存储
        std::shared_ptr<lens::graphics::Texture> m_currentFrame;
        std::atomic<bool> m_frameAvailable{ false };
        std::atomic<bool> m_isCapturing{ false };

        // 事件处理
        winrt::event_token m_frameArrivedToken;

        void OnFrameArrived(
            winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
            winrt::Windows::Foundation::IInspectable const& args
        );
    };
}