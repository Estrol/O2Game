/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Threads/Thread.h>

Thread::Thread()
{
    m_ThreadFunction = [](double) {};
    m_ThreadPreFunction = []() {};
    m_ThreadPosFunction = []() {};

    m_TimeWatch.SetTickRate(60.0);
}

Thread::Thread(std::function<void(double)> threadFunction, double tickRate) : Thread::Thread()
{
    m_ThreadFunction = threadFunction;
    m_TimeWatch.SetTickRate(tickRate);
}

Thread::Thread(
    std::function<void(void)>   preCallback,
    std::function<void(double)> onCallback,
    std::function<void(void)>   posCallback,
    double                      tickRate) : Thread::Thread()
{
    m_ThreadPreFunction = preCallback;
    m_ThreadFunction = onCallback;
    m_ThreadPosFunction = posCallback;

    m_TimeWatch.SetTickRate(tickRate);
}

void Thread::Start()
{
    m_Running = true;

    m_Thread = std::thread([&]() {
        try {
            m_ThreadPreFunction();

            while (m_Running) {
                Tick();
            }

            m_ThreadPosFunction();
        } catch (const std::exception) {
            m_Exception = std::current_exception();
        }
    });
}

void Thread::Stop()
{
    m_Running = false;
    m_Thread.join();
}

void Thread::Tick()
{
    m_Id = std::this_thread::get_id();
    double dt = m_TimeWatch.Tick();

    m_ThreadFunction(dt);

    if (m_Callback.size()) {
        for (auto callback : m_Callback) {
            callback();
        }
    }
}

void Thread::SetTickRate(double tickRate)
{
    m_TimeWatch.SetTickRate(tickRate);
}

std::thread::id Thread::GetId()
{
    return m_Id;
}

void Thread::AddCallback(std::function<void(void)> callback)
{
    m_Callback.push_back(callback);
}

std::exception_ptr Thread::GetException()
{
    return m_Exception;
}