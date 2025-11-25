#pragma once

#include "UIPanel.h"

namespace lens
{
    class DebugPanel : public UIPanel
    {
    private:
        bool m_visible = true;

    public:
        DebugPanel() = default;
        virtual ~DebugPanel() = default;

        const char* GetName() const override { return "Debug"; }
        bool IsVisible() const override { return m_visible; }
        void SetVisible(bool visible) override { m_visible = visible; }
        void Render() override;

        void Initialize() override;
        void Shutdown() override;
    };
}