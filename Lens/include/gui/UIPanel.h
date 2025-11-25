#pragma once

#include <string>

namespace lens
{
    class UIPanel
    {
    public:
        virtual ~UIPanel() = default;

        virtual const char* GetName() const = 0;
        virtual bool IsVisible() const = 0;
        virtual void SetVisible(bool visible) = 0;
        virtual void Render() = 0;

        virtual void Initialize() {}
        virtual void Shutdown() {}
    };
}