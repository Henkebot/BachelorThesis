#include "../pch.h"

#include "Input.h"

#include <Sandbox/Window/Window.h>

bool Input::IsKeyPressed(int _keyCode)
{
	return (GetAsyncKeyState(_keyCode) & 0x8000) != 0;
}

bool Input::IsKeyTyped(int _keyCode)
{
	return (GetAsyncKeyState(_keyCode) & 1) != 0;
}

bool Input::IsMouseButtonPressed(int _mouseCode)
{
	return (GetAsyncKeyState(_mouseCode) & 0x8000) != 0;
}

DirectX::XMINT2 Input::GetMousePosition()
{
	POINT point;
	GetCursorPos(&point);

	DirectX::XMINT2 pos;
	pos.x = static_cast<signed int>(point.x);
	pos.y = static_cast<signed int>(point.y);

	return pos;
}

DirectX::XMINT2 Input::GetMousePosition(const Window& _relativeWindow)
{
	HWND handle = _relativeWindow.getHandle();
	if(handle)
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(handle, &point);

		DirectX::XMINT2 pos;
		pos.x = static_cast<signed int>(point.x);
		pos.y = static_cast<signed int>(point.y);
		return pos;
	}
	else
	{
		//TODO(Henrik): Log this
		return DirectX::XMINT2();
	}
}
