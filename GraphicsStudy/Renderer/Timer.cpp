#include "Timer.h"
#include <Windows.h>
#include <iostream>

Utils::Timer::Timer():
	m_currTime(0),
	m_prevTime(0),
	m_stopTime(0),
	m_deltaTime(-1.f),
	m_secondPerCount(0),
	m_pausedTime(0)
{
	__int64 freq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	std::cout << freq;
	m_secondPerCount = 1.0/freq;

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_baseTime = currTime;
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
		m_prevTime = currTime;
		m_pausedTime += currTime - m_stopTime;
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
		m_currTime = currTime;

		m_deltaTime = delTime * m_secondPerCount;
		//std::cout << 1 / m_deltaTime << ' ';
		m_prevTime = m_currTime;
	}
}
