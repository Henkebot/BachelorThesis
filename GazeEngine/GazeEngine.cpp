#include "GazePCH.h"

#include "GazeEngine.h"

namespace GazeVar
{
	tobii_api_t* pApi;
	tobii_device_t* pDevice;

	int ScreenWidth, ScreenHeight;

	int GazeX, GazeY;

	HANDLE threadHandle;
	volatile LONG gThreadRunning;

} // namespace GazeVar

namespace GazeImpl
{
	void urlReciever(char const* pUrl, void* pUserData)
	{
		char* buffer = (char*)pUserData;
		if(*buffer != '\0')
			return;

		if(strlen(pUrl) < 256)
			strcpy_s(buffer, 256, pUrl);
	}

	void gazePointCallback(tobii_gaze_point_t const* pPoint, void* userData)
	{
		if(pPoint->validity == TOBII_VALIDITY_VALID)
		{
			GazeVar::GazeX = pPoint->position_xy[0] * (float)(GazeVar::ScreenWidth);
			GazeVar::GazeY = pPoint->position_xy[1] * (float)(GazeVar::ScreenHeight);
		}
	}

	DWORD gazeThreadProc(LPVOID pData)
	{
		while(InterlockedCompareExchange(&GazeVar::gThreadRunning, 0, 0))
		{
			tobii_error_t error;

			error = tobii_wait_for_callbacks(NULL, 1, &GazeVar::pDevice);
			assert(error == TOBII_ERROR_NO_ERROR || error == TOBII_ERROR_TIMED_OUT);

			error = tobii_device_process_callbacks(GazeVar::pDevice);
			assert(error == TOBII_ERROR_NO_ERROR);
		}
		return 0;
	}

} // namespace GazeImpl

namespace Gaze
{
	using namespace GazeVar;

	void Init(int _ScreenWidth, int _ScreenHeight)
	{
		GazeVar::ScreenWidth  = _ScreenWidth;
		GazeVar::ScreenHeight = _ScreenHeight;
		tobii_error_t error;

		error = tobii_api_create(&pApi, NULL, NULL);
		assert(error == TOBII_ERROR_NO_ERROR && "Failed to create tobii api");

		char url[256] = {};
		error		  = tobii_enumerate_local_device_urls(pApi, GazeImpl::urlReciever, url);
		assert(error == TOBII_ERROR_NO_ERROR && *url != '\0');

		error = tobii_device_create(pApi, url, &pDevice);
		assert(error == TOBII_ERROR_NO_ERROR);

		// Create the thread
		gThreadRunning = 1L;
		CreateThread(nullptr, 0, GazeImpl::gazeThreadProc, nullptr, 0, nullptr);

		error = tobii_gaze_point_subscribe(pDevice, GazeImpl::gazePointCallback, 0);
		assert(error == TOBII_ERROR_NO_ERROR);
	}

	void CleanUp()
	{
		InterlockedExchange(&gThreadRunning, 0);
		WaitForSingleObject(GazeVar::threadHandle, INFINITE);

		tobii_device_destroy(GazeVar::pDevice);
		tobii_api_destroy(GazeVar::pApi);
	}

	DirectX::XMINT2 GetGazePoint()
	{

		DirectX::XMINT2 coord = {GazeX, GazeY};

		return coord;
	}

} // namespace Gaze
