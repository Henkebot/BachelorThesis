// Sandbox.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <Sandbox/Input/Input.h>
#include <Sandbox/ScopedTimer.h>
#include <Sandbox/Window/Window.h>

const char* TestImages[] = {"../Resources/8bit/artificial.ppm",
							"../Resources/8bit/big_building.ppm",
							"../Resources/8bit/big_tree.ppm",
							"../Resources/8bit/deer.ppm",
							"../Resources/8bit/fireworks.ppm",
							"../Resources/8bit/flower_foveon.ppm",
							"../Resources/8bit/hdr.ppm",
							"../Resources/8bit/leaves_iso_1600.ppm",
							"../Resources/8bit/nightshot_iso_1600.ppm",
							"../Resources/8bit/spider_web.ppm",
							"../Resources/8bit/zone_plate.ppm"

};

int main()
{

	int currentImage = 0;

	DX12Wrap::InitD3D();

	Window window(VideoMode::GetDesktopMode(), L"Gaze compression");

	DX12Wrap::InitSwapChain(window.getHandle(), window.GetWidth(), window.GetHeight());

	Gaze::Init(window.GetWidth(), window.GetHeight());

	DX12Wrap::UseTexture("../Resources/8bit/zone_plate.ppm");

	DX12Wrap::Fullscreen(window.getHandle());

	int counter = 0;

	while(window.isOpen() && (false == Input::IsKeyTyped(VK_ESCAPE)))
	{
		//ScopedTimer r("MainLoop");

		window.pollEvents();

		if(Input::IsKeyTyped(VK_UP))
		{
			counter++;
			if(counter > 2)
				counter = 2;
		}
		else if(Input::IsKeyTyped(VK_DOWN))
		{
			counter--;
			if(counter < 0)
				counter = 0;
		}
		DX12Wrap::SetRadialFunction(counter);

		if(Input::IsKeyTyped(VK_RIGHT))
		{
			currentImage++;
			if(currentImage >= ARRAYSIZE(TestImages))
				currentImage = ARRAYSIZE(TestImages) - 1;

			DX12Wrap::UseTexture(TestImages[currentImage]);
		}
		else if(Input::IsKeyTyped(VK_LEFT))
		{
			currentImage--;
			if(currentImage < 0)
				currentImage = 0;

			DX12Wrap::UseTexture(TestImages[currentImage]);
		}

		//DX12Wrap::SetGazePoint(Gaze::GetGazePoint());
		DX12Wrap::SetGazePoint(Input::GetMousePosition());

		DX12Wrap::Render();
	}
	Gaze::CleanUp();
	DX12Wrap::CleanUp();
	return 0;

	system("pause");
}