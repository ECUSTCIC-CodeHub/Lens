#pragma once
#include <Windows.h>

namespace lens
{
	class Application
	{
	public:
		Application(HINSTANCE hInstance, int mCmdShow, 
			const wchar_t* className = L"LensWindowClass", const wchar_t* title = L"Lens Application");

		~Application();

		bool Create(int width = 800, int height = 600);

        int Run();

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HINSTANCE m_hInstance;
		int m_nCmdShow;
		HWND m_hwnd;
		std::wstring m_className;
		std::wstring m_title;
		bool isExit;
	};
}