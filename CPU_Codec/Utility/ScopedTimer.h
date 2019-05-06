#pragma once
#include <Windows.h>
#include <stdio.h>
typedef float real32;
typedef double real64;


class ScopedTimer
{
private:
	LARGE_INTEGER m_StartingTime;
	LARGE_INTEGER m_EndingTime;
	LARGE_INTEGER m_Frequency;
	const char* m_Name;
public:
	ScopedTimer(const char* Name) : m_Name(Name)
	{
		QueryPerformanceFrequency(&m_Frequency);
		QueryPerformanceCounter(&m_StartingTime);

	}

	~ScopedTimer()
	{

		QueryPerformanceCounter(&m_EndingTime);
		LARGE_INTEGER ElapsedMicroseconds;
		ElapsedMicroseconds.QuadPart = m_EndingTime.QuadPart - m_StartingTime.QuadPart;

		ElapsedMicroseconds.QuadPart *= 1000LL;
		float MSPerFrame = ((real32)ElapsedMicroseconds.QuadPart / (real32)m_Frequency.QuadPart);
		m_StartingTime = m_EndingTime;

		char Buffer[256];
		sprintf_s(Buffer, 256, "%s\t%fm/s\n", m_Name, MSPerFrame);
		OutputDebugStringA(Buffer);

	}
};
