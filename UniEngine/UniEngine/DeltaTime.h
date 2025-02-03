#pragma once

#include <chrono>

class DeltaTime
{
public:
	float CalculateDeltaTime();
private:
	float m_DeltaTime = 0;
	std::chrono::time_point<std::chrono::steady_clock> m_LastRecordedTime;
};

