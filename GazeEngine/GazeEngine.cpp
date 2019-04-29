#include "GazePCH.h"

#include "GazeEngine.h"

namespace GazeVar
{
	tobii_api_t* pApi;
	tobii_device_t* pDevice;

	int ScreenWidth, ScreenHeight;

	int GazeX, GazeY;
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

} // namespace GazeImpl

namespace Gaze
{
	using namespace GazeVar;

	void Init(int _ScreenWidth, int _ScreenHeight)
	{
		GazeVar::ScreenWidth = _ScreenWidth;
		GazeVar::ScreenHeight = _ScreenHeight;
		tobii_error_t error;

		error = tobii_api_create(&pApi, NULL, NULL);
		assert(error == TOBII_ERROR_NO_ERROR && "Failed to create tobii api");

		char url[256] = {};
		error		  = tobii_enumerate_local_device_urls(pApi, GazeImpl::urlReciever, url);
		assert(error == TOBII_ERROR_NO_ERROR && *url != '\0');

		error = tobii_device_create(pApi, url, &pDevice);
		assert(error == TOBII_ERROR_NO_ERROR);

		error = tobii_gaze_point_subscribe(pDevice, GazeImpl::gazePointCallback, 0);
		assert(error == TOBII_ERROR_NO_ERROR);
	}

	void Update()
	{
		tobii_error_t error;

		error = tobii_wait_for_callbacks(NULL, 1, &pDevice);
		assert(error == TOBII_ERROR_NO_ERROR || error == TOBII_ERROR_TIMED_OUT);

		error = tobii_device_process_callbacks(pDevice);
		assert(error == TOBII_ERROR_NO_ERROR);
	}

	DirectX::XMINT2 GetGazePoint()
	{

		DirectX::XMINT2 coord = {GazeX, GazeY};

		return coord;
	}

} // namespace Gaze
