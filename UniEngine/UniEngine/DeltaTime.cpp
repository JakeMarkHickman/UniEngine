#include "DeltaTime.h"

float DeltaTime::CalculateDeltaTime()
{
    static bool DoOnce = false;

    if (!DoOnce)
    {
        DoOnce = true;
        m_LastRecordedTime = std::chrono::steady_clock::now();
        return 0.0f;
    }

    auto curTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = curTime - m_LastRecordedTime;

    m_DeltaTime = elapsed.count();

    m_LastRecordedTime = curTime;
    return m_DeltaTime;
}
