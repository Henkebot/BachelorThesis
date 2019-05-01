#pragma once
#include <Windows.h>
#include <stdio.h>
typedef float real32;
typedef double real64;

class Timer
{
private:
	LARGE_INTEGER m_StartingTime;
	LARGE_INTEGER m_EndingTime;
	LARGE_INTEGER m_Frequency;
	const char* m_Name;

public:
	Timer()
	{
		QueryPerformanceFrequency(&m_Frequency);
		QueryPerformanceCounter(&m_StartingTime);
	}

	float RestartAndGetElapsedTimeMS()
	{
		QueryPerformanceCounter(&m_EndingTime);
		LARGE_INTEGER ElapsedMicroseconds;
		ElapsedMicroseconds.QuadPart = m_EndingTime.QuadPart - m_StartingTime.QuadPart;

		ElapsedMicroseconds.QuadPart *= 1000LL;
		float MSPerFrame = ((real32)ElapsedMicroseconds.QuadPart / (real32)m_Frequency.QuadPart);
		m_StartingTime   = m_EndingTime;

		return MSPerFrame;
	}

	
};
