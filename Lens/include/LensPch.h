#pragma once

// WinRT
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.Web.Syndication.h>

#include <DispatcherQueue.h>
#include <windows.ui.composition.interop.h>

#include <Windows.h>

// Standard Library
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>
#include <chrono>

// DirectX
#include <d3d11_1.h>
#include <dxgi1_2.h>

// ImGui (excluding from PCH for ImGui files themselves)
#ifndef IMGUI_USER_CONFIG
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#endif

#include "Log.h"