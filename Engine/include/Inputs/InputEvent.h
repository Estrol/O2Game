#pragma once
#include <functional>

typedef std::function<void(int)> m_event_callback;

class InputEvent
{
public:
    bool IsActive();
    void Disconnect();
    void CreateCallback(m_event_callback callback, int index);

private:
    bool             m_activated;
    int              m_inputIndex;
    m_event_callback m_callback;
};