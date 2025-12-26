#include "LensPch.h"
#include "capturer/WGCCapturer.h"
#include <windows.graphics.capture.interop.h>
#include <Windows.Graphics.DirectX.Direct3D11.Interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>

namespace lens::capturer
{
    WGCCapturer::WGCCapturer(lens::graphics::GraphicsDevice* device)
        : m_device(device)
    {
        init_apartment(winrt::apartment_type::single_threaded);
    }

    WGCCapturer::~WGCCapturer()
    {
        Shutdown();
    }

    bool WGCCapturer::Initialize(const CaptureDesc& desc)
    {
        m_desc = desc;
        LOG_INFO("WGCCapturer initialized with format: {}, fps: {}", static_cast<int>(desc.format), desc.frameRate);
        return true;
    }

    void WGCCapturer::Shutdown()
    {
        StopCapture();

        if (m_frameArrivedToken)
        {
            if (m_framePool)
            {
                m_framePool.FrameArrived(m_frameArrivedToken);
            }
            m_frameArrivedToken = winrt::event_token{};
        }

        m_framePool = nullptr;
        m_session = nullptr;
        m_captureItem = nullptr;
        m_currentFrame = nullptr;

        LOG_INFO("WGCCapturer shut down");
    }

    bool WGCCapturer::StartCapture(HWND window)
    {
        if (m_isCapturing)
        {
            LOG_WARN("Already capturing");
            return false;
        }

        try
        {
            // 创建捕获项 - 使用 Windows.Graphics.Capture.Interop 命名空间
            auto interop_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
            winrt::com_ptr<IGraphicsCaptureItemInterop> interop;
            interop_factory.as(interop);

            winrt::com_ptr<IInspectable> inspectable;
            HRESULT hr = interop->CreateForWindow(
                window,
                winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(),
                reinterpret_cast<void**>(inspectable.put()));

            if (FAILED(hr))
            {
                LOG_ERROR("Failed to create capture item for window: 0x{:X}", hr);
                return false;
            }

            m_captureItem = inspectable.as<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
            LOG_INFO("Capture item created: {}x{}", m_captureItem.Size().Width, m_captureItem.Size().Height);

            // 创建 D3D11 设备和帧池
            auto d3dDevice = m_device->GetDevice();
            winrt::com_ptr<IDXGIDevice> dxgiDevice;
            hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), dxgiDevice.put_void());

            if (FAILED(hr))
            {
                LOG_ERROR("Failed to query DXGI device: 0x{:X}", hr);
                return false;
            }

            winrt::com_ptr<IInspectable> deviceInspectable;
            hr = CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), deviceInspectable.put());

            if (FAILED(hr))
            {
                LOG_ERROR("Failed to create Direct3D11 device from DXGI: 0x{:X}", hr);
                return false;
            }

            auto winrtDevice = deviceInspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
            auto pixelFormat = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;

            m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
                winrtDevice,
                pixelFormat,
                2, // 缓冲帧数
                m_captureItem.Size());

            m_session = m_framePool.CreateCaptureSession(m_captureItem);

            // 注册帧到达事件
            m_frameArrivedToken = m_framePool.FrameArrived(
                { this, &WGCCapturer::OnFrameArrived });

            // 开始捕获
            m_session.StartCapture();
            m_isCapturing = true;
            m_frameAvailable = false;

            LOG_INFO("WGC capture started successfully");
            return true;
        }
        catch (const winrt::hresult_error& e)
        {
            LOG_ERROR("WGC capture failed: {}", winrt::to_string(e.message()));
            return false;
        }
    }

    void WGCCapturer::StopCapture()
    {
        if (!m_isCapturing)
            return;

        if (m_session)
        {
            m_session.Close();
            m_session = nullptr;
        }

        if (m_framePool)
        {
            m_framePool.Close();
            m_framePool = nullptr;
        }

        m_isCapturing = false;
        m_frameAvailable = false;
        LOG_INFO("WGC capture stopped");
    }

    std::shared_ptr<lens::graphics::Texture> WGCCapturer::GetLatestFrame()
    {
        if (!m_frameAvailable || !m_currentFrame)
            return nullptr;

        m_frameAvailable = false;
        return m_currentFrame;
    }

    void WGCCapturer::OnFrameArrived(
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
        winrt::Windows::Foundation::IInspectable const& args)
    {
        auto frame = sender.TryGetNextFrame();
        if (!frame)
        {
            return;
        }

        try
        {
            auto frameSurface = frame.Surface();
            auto d3dSurface = frameSurface.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface>();

            winrt::com_ptr<IInspectable> surfaceInspectable = d3dSurface.as<IInspectable>();
            winrt::com_ptr<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess> dxgiAccess;
            surfaceInspectable.as(dxgiAccess);

            winrt::com_ptr<ID3D11Texture2D> frameTexture;
            HRESULT hr = dxgiAccess->GetInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(frameTexture.put()));

            if (SUCCEEDED(hr) && frameTexture)
            {
                auto texture = std::make_shared<lens::graphics::Texture>();
                if (texture->CreateFromD3DTexture(m_device, frameTexture.get()))
                {
                    m_currentFrame = texture;
                    m_frameAvailable = true;
                }
            }
        }
        catch (const winrt::hresult_error& e)
        {
            LOG_ERROR("Error processing frame: {}", winrt::to_string(e.message()));
        }
    }

    std::vector<WGCCapturer::CaptureSource> WGCCapturer::EnumerateWindows()
    {
        std::vector<CaptureSource> sources;

        struct EnumData
        {
            std::vector<CaptureSource>* sources;
        } data = { &sources };

        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            EnumData* data = reinterpret_cast<EnumData*>(lParam);

            if (!IsWindowVisible(hwnd))
                return TRUE;

            if (GetWindowTextLength(hwnd) == 0)
                return TRUE;

            RECT rect;
            GetWindowRect(hwnd, &rect);

            WCHAR title[256];
            GetWindowText(hwnd, title, 256);

            CaptureSource source;
            source.windowHandle = hwnd;
            source.windowTitle = title;
            source.windowRect = rect;

            data->sources->push_back(source);
            return TRUE;
        }, reinterpret_cast<LPARAM>(&data));

        return sources;
    }
}