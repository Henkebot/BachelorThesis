#pragma once
#include "GazePCH.h"

namespace Gaze
{
	// If the eye tracker fails to initialize, then the mouse cursor will be used.

	void Init(int _ScreenWidth, int _ScreenHeight);

	void Update();

	DirectX::XMINT2 GetGazePoint();
}