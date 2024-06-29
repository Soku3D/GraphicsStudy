#pragma once

namespace Utils {
	class Timer {
	public:
		Timer();
		~Timer();

		void Start();
		void Stop();
		void Tick();

		double GetDeltaTime() const { return m_deltaTime; }
		inline double GetElapsedTime() const { return m_secondPerCount * (m_currTime - m_baseTime - m_pausedTime); }
		inline double GetFrameRate() const { return 1 / m_deltaTime; }
	private:
		__int64 m_currTime;
		__int64 m_prevTime;
		__int64 m_stopTime;
		__int64 m_pausedTime;
		__int64 m_baseTime;

		double m_secondPerCount;
		double m_deltaTime;
		bool m_stoped = false;
	};
}

