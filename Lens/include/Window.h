#pragma once
#include <Window.h>

namespace lens
{
	class Window
	{
	public:
		Window(HINSTANCE hInstance, int mCmdShow, 
			const wchar_t* className = L"LensWindowClass", const wchar_t* title = L"Lens Application");

		~Window();


	private:
		HINSTANCE m_hInstance;
		int m_nCmdShow;
		HWND m_hwnd;
		std::wstring m_className;
		std::wstring m_title;
	};
}