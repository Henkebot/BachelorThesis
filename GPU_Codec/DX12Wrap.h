#pragma once
#pragma once
#pragma once
#include "GPUPch.h"

namespace DX12Wrap
{

	void InitD3D(void);

	void InitSwapChain(HWND _WindowHandle, unsigned int _width, unsigned int _height);

	void Fullscreen(HWND _WindowHandle);

	// Load texture is loading and changing the display texture on the screen.
	void LoadTexture(const char* _pTexturePath);

	void GazePoint(DirectX::XMINT2 _Coord);

	void Render(void);

	void CleanUp(void);

}; // namespace DX12Wrap
