#pragma once
#include "VideoMode.h"
#include "../pch.h"
class Window
{
public:
	/*
		Construct a window with its attributes

		Params:
		mode		Video mode
		title		Title of the window
	*/
	Window(const VideoMode& _mode, const wchar_t* _title);

	virtual ~Window();

	void setSize(unsigned int _width, unsigned int _height);
	DirectX::XMUINT2 getSize() const;

	UINT GetWidth() const;
	UINT GetHeight() const;


	bool isOpen() const;
	void closeWindow();
	HWND getHandle() const;

	void pollEvents();

public:
	// Used to process Windows process events, should be private
	bool processEvent(UINT _message, WPARAM _wParam, LPARAM _lParam);

private:
	bool registerWindowClass();

	static LRESULT CALLBACK preWinProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND m_handle;
	UINT m_windowStyle;

	bool m_running;
	bool m_fullscreenMode;
	RECT m_windowRect;
};