#pragma once
#include "../pch.h"

class Window;
namespace Input
{
	bool IsKeyPressed(int _keyCode);
	bool IsKeyTyped(int _keyCode);

	bool IsMouseButtonPressed(int _mouseCode);

	DirectX::XMINT2 GetMousePosition();
	DirectX::XMINT2 GetMousePosition(const Window& _relativeWindow);

} // namespace Input