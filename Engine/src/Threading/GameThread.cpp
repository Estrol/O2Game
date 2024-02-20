#include "Rendering/Threading/GameThread.h"

GameThread::GameThread()
{
    m_run = false;
    m_background = false;
}

GameThread::~GameThread()
{
    if (m_run) {
        Stop();
    }
}

void GameThread::Run(std::function<void()> callback, bool background)
{
    m_main_cb = callback;
    m_run = true;
    m_background = background;

    if (background) {
        m_thread = std::thread([this] {
            while (m_run) {
                std::function<void()> main_cb;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    main_cb = m_main_cb;
                }
                if (main_cb) {
                    main_cb();
                }

                std::function<void()> queue_cb;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (!m_queue_cb.empty()) {
                        queue_cb = m_queue_cb.front();
                        m_queue_cb.pop_back();
                    }
                }
                if (queue_cb) {
                    queue_cb();
                }
            }
            });
    }
}

void GameThread::Update()
{
    if (!m_background) {
        std::function<void()> main_cb;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            main_cb = m_main_cb;
        }
        if (main_cb) {
            main_cb();
        }

        std::function<void()> queue_cb;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_queue_cb.empty()) {
                queue_cb = m_queue_cb.back();
                m_queue_cb.pop_back();
            }
        }
        if (queue_cb) {
            queue_cb();
        }
    }
}

void GameThread::QueueAction(std::function<void()> callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue_cb.push_back(callback);
}

void GameThread::Stop()
{
    if (m_background) {
        m_run = false;
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
}
