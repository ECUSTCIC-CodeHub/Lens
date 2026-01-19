#pragma once

#include "UIPanel.h"
#include "capturer/WGCCapturer.h"

namespace lens
{
    class CapturePanel : public UIPanel
    {
    private:
        bool m_visible = true;
        capturer::WGCCapturer* m_capturer;
        std::shared_ptr<lens::graphics::Texture> m_lastFrame; // Cache the latest frame

    public:
        CapturePanel();
        virtual ~CapturePanel() = default;

        void SetCapturer(capturer::WGCCapturer* capturer) { m_capturer = capturer; }

        const char* GetName() const override { return "Capture"; }
        bool IsVisible() const override { return m_visible; }
        void SetVisible(bool visible) override { m_visible = visible; }
        void Render() override;

        void Initialize() override;
        void Shutdown() override;
    };
}
