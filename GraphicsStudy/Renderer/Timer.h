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
	private:
		__int64 m_currTime;
		__int64 m_prevTime;
		__int64 m_stopTime;
		__int64 m_elapsedTime;
		
		double m_invFreqency;
		double m_deltaTime;
		bool m_stoped = true;
	};
}

