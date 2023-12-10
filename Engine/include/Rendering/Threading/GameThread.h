#pragma once
#include <functional>
#include <thread>
#include <vector>

class GameThread
{
public:
    GameThread();
    ~GameThread();

    void Run(std::function<void()> callback, bool background);
    void QueueAction(std::function<void()> callback);

    void Update();
    void Stop();

private:
    bool m_run;
    bool m_background;

    std::thread m_thread;

    std::function<void()>              m_main_cb;
    std::vector<std::function<void()>> m_queue_cb;
};