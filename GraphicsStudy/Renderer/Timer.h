#pragma once

namespace Utils {
	class Timer {
	public:
		Timer();
		~Timer();

		void Start();
		void Stop();
		void Tick();
		void Reset();

		inline double GetDeltaTime() const { return m_deltaTime; }
		inline double GetElapsedTime() const { return m_secondPerCount * (m_currTime - m_baseTime - m_pausedTime); }
		inline double GetFrameRate() const { return 1 / m_deltaTime; }

		inline __int64 GetCurrtTime() const { return m_currTime; }
		inline __int64 GetPrevTime() const { return m_prevTime; }
		inline __int64 GetPausedTime() const { return m_pausedTime; }
		inline __int64 GetBaseTime() const { return m_baseTime; }

	private:
		__int64 m_currTime;
		__int64 m_prevTime;
		__int64 m_stopTime;
		__int64 m_pausedTime;
		__int64 m_baseTime;

		double m_secondPerCount;
		double m_deltaTime;
		bool m_stoped = true;
	};
}

