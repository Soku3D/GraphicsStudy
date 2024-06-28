#include "Timer.h"
#include <Windows.h>

Utils::Timer::Timer():
	m_currTime(0),
	m_prevTime(0),
	m_stopTime(0),
	m_deltaTime(-1.f),
	m_invFreqency(0)
{
	__int64 freq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	m_invFreqency = 1.0/freq;
}
Utils::Timer::~Timer()
{
}
void Utils::Timer::Start()
{
	if (m_stoped) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_currTime = currTime;

		m_stoped = false;
	}
}
void Utils::Timer::Stop()
{
	if (!m_stoped) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_stopTime = currTime;
		m_stoped = true;
	}

}

void Utils::Timer::Tick()
{
	if (!m_stoped) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		__int64 delTime = currTime - m_prevTime;
		//m_elapsedTime += delTime;

		m_deltaTime = delTime * m_invFreqency;
		m_prevTime = currTime;
	}
}
