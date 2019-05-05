// Sandbox.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <Sandbox/Input/Input.h>
#include <Sandbox/ScopedTimer.h>
#include <Sandbox/Timer.h>
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
							"../Resources/8bit/spider_web.ppm"

};

enum class STAGE
{
	STAGE_INTRO,
	STAGE_OUTRO,
	STAGE_PRESENT,
	STAGE_PREPARE,
	STAGE_SELECTION
} gCurrentStage;

struct GAZE_Setting
{
	enum GAZE_FUNCTION
	{
		GAZE_FUNCTION_LIN = 0,
		GAZE_FUNCTION_FOV
	} radialFunction;

	float circlePercentage;
	float innerQualityPercentage;
};

enum DISPLAY_TYPE
{
	DISPLAY_TYPE_GAZE,
	DISPLAY_TYPE_RAW
};

void SetupSettings(GAZE_Setting settings[10], int imageIndexes[10]);

void LogResult(std::ofstream& file,
			   int imageIndexes[10],
			   int totalSimulationStep,
			   DISPLAY_TYPE displayType[2],
			   int choice,
			   GAZE_Setting settings[10]);

int main()
{
	srand(static_cast<unsigned int>(time(0)));

	// Settings table
	GAZE_Setting settings[10];
	int imageIndexes[10];
	SetupSettings(settings, imageIndexes);

	gCurrentStage = STAGE::STAGE_INTRO;

	DX12Wrap::InitD3D();

	Window window(VideoMode::GetDesktopMode(), L"Gaze compression");

	DX12Wrap::InitSwapChain(window.getHandle(), window.GetWidth(), window.GetHeight());

	Gaze::Init(window.GetWidth(), window.GetHeight());

	DX12Wrap::Fullscreen(window.getHandle());

	Timer t;

	float time = 0;

	DISPLAY_TYPE displayType[2];
	displayType[0]			= DISPLAY_TYPE_GAZE;
	displayType[1]			= DISPLAY_TYPE_RAW;
	int totalSimulationStep = 0;
	int simulationStep		= 0;

	std::ofstream output;
	output.open("Results.txt", std::ofstream::out | std::ofstream::app);
	output << "Start of test------------------------------------------\n"
		   << "Image\tRAW\tGAZE\tLinear\tFOV\tRadius 1\tRadius 0.8\tRadius 0.6\tQuality 1\tQuality "
			  "0.8\tQuality 0.6\n";

	while(window.isOpen() && (false == Input::IsKeyTyped(VK_ESCAPE)))
	{
		//ScopedTimer r("MainLoop");

		window.pollEvents();

		DX12Wrap::SetGazePoint(Gaze::GetGazePoint());
		DX12Wrap::SetRadialFunction(settings[totalSimulationStep].radialFunction);
		DX12Wrap::SetRadiusSetting(settings[totalSimulationStep].circlePercentage,
								   settings[totalSimulationStep].innerQualityPercentage);

		// Simple StateMachine
		switch(gCurrentStage)
		{
		case STAGE::STAGE_INTRO:
		{
			DX12Wrap::RenderText(L"Press ENTER to start the study");
			if(Input::IsKeyTyped(VK_RETURN))
			{
				gCurrentStage = STAGE::STAGE_PREPARE;
				DX12Wrap::UseTexture(TestImages[imageIndexes[totalSimulationStep]]);
				time = 0.0f;
				t.RestartAndGetElapsedTimeMS();
			}
		}
		break;

		case STAGE::STAGE_OUTRO:
		{
			DX12Wrap::RenderText(L"Thanks for participating!\n\nPress ENTER to exit\n");
			if(Input::IsKeyTyped(VK_RETURN))
				window.closeWindow();
		}
		break;

		case STAGE::STAGE_PREPARE:
		{
			time += t.RestartAndGetElapsedTimeMS() * (Input::IsKeyPressed(VK_SHIFT) ? 10.0f : 1.0f);
			if(time >= 5000.0f)
			{
				gCurrentStage = STAGE::STAGE_PRESENT;

				time = 0.0f;
				t.RestartAndGetElapsedTimeMS();
				break;
			}
			WCHAR buffer[256];
			wsprintfW(buffer, L"Image [%i] in %i", (simulationStep + 1), 5 - int(time / 1000));
			DX12Wrap::RenderText(buffer);
		}
		break;

		case STAGE::STAGE_PRESENT:
		{
			time += t.RestartAndGetElapsedTimeMS() * (Input::IsKeyPressed(VK_SHIFT) ? 10.0f : 1.0f);
			if(time >= 5000.0f)
			{
				time = 0.0f;
				t.RestartAndGetElapsedTimeMS();
				simulationStep++;
				gCurrentStage = STAGE::STAGE_PREPARE;
			}
			if(simulationStep == 2)
			{
				gCurrentStage  = STAGE::STAGE_SELECTION;
				simulationStep = 0;

				break;
			}

			switch(displayType[simulationStep])
			{
			case DISPLAY_TYPE_GAZE:
			{
				DX12Wrap::RenderGaze();
			}
			break;

			case DISPLAY_TYPE_RAW:
			{
				DX12Wrap::RenderRAW();
			}
			break;
			}
		}
		break;

		case STAGE::STAGE_SELECTION:
		{

			WCHAR buffer[256];
			wsprintfW(buffer,
					  L"[%i/10]\n\nWhich image had the best quality?\n\nPress 1 for the "
					  L"first image\nOR\nPress 2 for the second image\n",
					  totalSimulationStep + 1);
			DX12Wrap::RenderText(buffer);

			int choice = -1;

			if(Input::IsKeyTyped(VK_NUMPAD1) || Input::IsKeyTyped('1'))
			{
				choice = 0;
			}
			else if(Input::IsKeyTyped(VK_NUMPAD2) || Input::IsKeyTyped('2'))
			{
				choice = 1;
			}

			if(choice != -1)
			{

				// Now do we need to log the results
				LogResult(output, imageIndexes, totalSimulationStep, displayType, choice, settings);

				// Shuffle the display type for next round
				if(rand() % 2 == 0)
				{
					std::swap(displayType[0], displayType[1]);
				}

				totalSimulationStep++;
				if(totalSimulationStep == 10)
				{
					gCurrentStage = STAGE::STAGE_OUTRO;
					break;
				}
				DX12Wrap::RenderText(L"Loading next image");
				DX12Wrap::UseTexture(TestImages[imageIndexes[totalSimulationStep]]);

				gCurrentStage = STAGE::STAGE_PREPARE;
				time		  = 0.0f;
				t.RestartAndGetElapsedTimeMS();
			}
		}
		break;
		}
	}

	output << "------------------------------------------End of test\n";

	Gaze::CleanUp();
	DX12Wrap::CleanUp();
	return 0;

	system("pause");
}

void LogResult(std::ofstream& file,
			   int imageIndexes[10],
			   int totalSimulationStep,
			   DISPLAY_TYPE displayType[2],
			   int choice,
			   GAZE_Setting settings[10])
{
	file << (TestImages[imageIndexes[totalSimulationStep]] + 18) << "\t"
		 << bool(displayType[choice] == DISPLAY_TYPE_RAW) << "\t"
		 << bool(displayType[choice] == DISPLAY_TYPE_GAZE) << "\t";

	file << bool(settings[totalSimulationStep].radialFunction == GAZE_Setting::GAZE_FUNCTION_LIN)
		 << "\t"
		 << bool(settings[totalSimulationStep].radialFunction == GAZE_Setting::GAZE_FUNCTION_FOV)
		 << "\t" << bool(settings[totalSimulationStep].circlePercentage == 1.0f) << "\t"
		 << bool(settings[totalSimulationStep].circlePercentage == 0.8f) << "\t"
		 << bool(settings[totalSimulationStep].circlePercentage == 0.6f) << "\t"
		 << bool(settings[totalSimulationStep].innerQualityPercentage == 1.0f) << "\t"
		 << bool(settings[totalSimulationStep].innerQualityPercentage == 0.8f) << "\t"
		 << bool(settings[totalSimulationStep].innerQualityPercentage == 0.6f);

	file << "\n";
}

void SetupSettings(GAZE_Setting settings[10], int imageIndexes[10])
{
	settings[0].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_LIN;
	settings[0].circlePercentage	   = 1.0f;
	settings[0].innerQualityPercentage = 1.0f;

	settings[1].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_LIN;
	settings[1].circlePercentage	   = 0.8f;
	settings[1].innerQualityPercentage = 1.0f;

	settings[2].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_LIN;
	settings[2].circlePercentage	   = 0.6f;
	settings[2].innerQualityPercentage = 1.0f;

	settings[3].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_LIN;
	settings[3].circlePercentage	   = 1.0f;
	settings[3].innerQualityPercentage = 0.8f;

	settings[4].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_LIN;
	settings[4].circlePercentage	   = 1.0f;
	settings[4].innerQualityPercentage = 0.6f;

	settings[5].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_FOV;
	settings[5].circlePercentage	   = 1.0f;
	settings[5].innerQualityPercentage = 1.0f;

	settings[6].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_FOV;
	settings[6].circlePercentage	   = 0.8f;
	settings[6].innerQualityPercentage = 1.0f;

	settings[7].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_FOV;
	settings[7].circlePercentage	   = 0.6f;
	settings[7].innerQualityPercentage = 1.0f;

	settings[8].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_FOV;
	settings[8].circlePercentage	   = 1.0f;
	settings[8].innerQualityPercentage = 0.8f;

	settings[9].radialFunction		   = GAZE_Setting::GAZE_FUNCTION_FOV;
	settings[9].circlePercentage	   = 1.0f;
	settings[9].innerQualityPercentage = 0.6f;

	for(int i = 0; i < 10; i++)
		imageIndexes[i] = i;

	// Shuffle the settings
	std::random_shuffle(settings, settings + 9);

	// Shuffle the image indexes

	std::random_shuffle(imageIndexes, imageIndexes + 9);
}
