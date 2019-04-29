#include "../pch.h"
#include "Window.h"

namespace
{
	const wchar_t* className = L"window";
}

////////////////////////////////////////////////////
Window::Window(const VideoMode& _mode, const wchar_t* _title)
	: m_running(true)
	, m_fullscreenMode(false)
{
	registerWindowClass();

	HDC screenDC = GetDC(NULL);
	int left	 = (GetDeviceCaps(screenDC, HORZRES) - static_cast<int>(_mode.GetWidth())) >> 1;
	int top		 = (GetDeviceCaps(screenDC, VERTRES) - static_cast<int>(_mode.GetHeight())) >> 1;
	int width	= _mode.GetWidth();
	int height   = _mode.GetHeight();
	ReleaseDC(NULL, screenDC);

	m_windowStyle = WS_OVERLAPPEDWINDOW;

	RECT rectangle = {0, 0, width, height};
	AdjustWindowRect(&rectangle, m_windowStyle, false);
	width  = rectangle.right - rectangle.left;
	height = rectangle.bottom - rectangle.top;

	m_handle = CreateWindowW(className,
							 _title,
							 m_windowStyle,
							 left,
							 top,
							 width,
							 height,
							 NULL,
							 NULL,
							 GetModuleHandle(NULL),
							 this);

	assert(m_handle != NULL);

	setSize(width, height);

	ShowWindow(m_handle, SW_SHOW);
}

////////////////////////////////////////////////////
Window::~Window()
{
	DestroyWindow(m_handle);
	UnregisterClassW(className, GetModuleHandle(NULL));
}

////////////////////////////////////////////////////
void Window::pollEvents()
{
	MSG message;
	while(PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}

////////////////////////////////////////////////////
void Window::setSize(unsigned int _width, unsigned int _height)
{
	RECT rectangle = {0, 0, static_cast<long>(_width), static_cast<long>(_height)};
	AdjustWindowRect(&rectangle, GetWindowLong(m_handle, GWL_STYLE), false);
	int width  = rectangle.right - rectangle.left;
	int height = rectangle.bottom - rectangle.top;

	SetWindowPos(m_handle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

////////////////////////////////////////////////////
DirectX::XMUINT2 Window::getSize() const
{
	RECT rect;
	GetClientRect(m_handle, &rect);

	DirectX::XMUINT2 size;
	size.x = rect.right - rect.left;
	size.y = rect.bottom - rect.top;

	return size;
}

UINT Window::GetWidth() const
{
	return getSize().x;
}

UINT Window::GetHeight() const
{
	return getSize().y;
}

////////////////////////////////////////////////////
bool Window::isOpen() const
{
	return m_running;
}

////////////////////////////////////////////////////
void Window::closeWindow()
{
	m_running = false;
}

////////////////////////////////////////////////////
HWND Window::getHandle() const
{
	return m_handle;
}

////////////////////////////////////////////////////
bool Window::registerWindowClass()
{
	WNDCLASSW windowClass	 = {};
	windowClass.style		  = 0;
	windowClass.lpfnWndProc   = &Window::preWinProc;
	windowClass.cbClsExtra	= 0;
	windowClass.cbWndExtra	= 0;
	windowClass.hInstance	 = GetModuleHandleW(NULL);
	windowClass.hIcon		  = NULL;
	windowClass.hCursor		  = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName  = NULL;
	windowClass.lpszClassName = className;
	return RegisterClassW(&windowClass);
}

////////////////////////////////////////////////////
bool Window::processEvent(UINT _message, WPARAM _wParam, LPARAM _lParam)
{

	switch(_message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		m_running = false;
		return true;
	}
	/*case WM_GETMINMAXINFO:
	{
		MINMAXINFO* pMinMaxInfo		  = (MINMAXINFO*)_lParam;
		pMinMaxInfo->ptMinTrackSize.x = 800;
		pMinMaxInfo->ptMinTrackSize.y = 480;
		pMinMaxInfo->ptMaxTrackSize.x = LONG_MAX;
		pMinMaxInfo->ptMaxTrackSize.y = LONG_MAX;
		return true;
	}*/
	case WM_SIZE:
	{
		RECT clientRect = {};
		GetClientRect(m_handle, &clientRect);
	}
	}
	return false;
}

////////////////////////////////////////////////////
LRESULT CALLBACK procProxy(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* pWindow = (Window*)GetWindowLongPtrW(handle, GWLP_USERDATA);
	if(pWindow->processEvent(message, wParam, lParam))
	{
		return 0;
	}

	return DefWindowProcW(handle, message, wParam, lParam);
}

////////////////////////////////////////////////////
LRESULT CALLBACK Window::preWinProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_CREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;

		SetWindowLongPtrW(handle, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
		SetWindowLongPtrW(handle, GWLP_WNDPROC, (LONG_PTR)procProxy);

		return 0;
	}
	return DefWindowProcW(handle, message, wParam, lParam);
}
