#include "LensPch.h"
#include "Window.h"

namespace lens
{
    Window::Window(HINSTANCE hInstance, int mCmdShow, 
        const wchar_t* className = L"LensWindowClass", const wchar_t* title = L"Lens Application")
        : m_hInstance(hInstance), 
          m_nCmdShow(mCmdShow), 
          m_className(className), 
          m_title(title)
    {
        init_apartment(winrt::apartment_type::single_threaded);
    }

    Window::~Window()
    {
    }
}
