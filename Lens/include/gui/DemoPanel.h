#pragma once

#include "UIPanel.h"

namespace lens
{
    class DemoPanel : public UIPanel
    {
    private:
        bool m_visible = true;
        float m_color[4] = { 0.0f, 0.5f, 0.8f, 1.0f };
        int m_counter = 0;

    public:
        DemoPanel();
        virtual ~DemoPanel() = default;

        const char* GetName() const override { return "Demo"; }
        bool IsVisible() const override { return m_visible; }
        void SetVisible(bool visible) override { m_visible = visible; }
        void Render() override;

        void Initialize() override;
        void Shutdown() override;
    };
}