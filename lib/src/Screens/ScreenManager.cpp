/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Exceptions/EstException.h>
#include <Game.h>
#include <Inputs/InputManager.h>
#include <Screens/ScreenManager.h>
using namespace Screens;

static Manager *instance = nullptr;

void Manager::Init(Game *game)
{
    m_Game = game;
}

void Manager::Update(double delta)
{
    if (m_NextScreen != nullptr) {
        if (m_CurrentScreen != nullptr) {
            m_CurrentScreen->Detach();
        }
        m_CurrentScreen = m_NextScreen;
        m_CurrentScreen->Attach();
        m_NextScreen = nullptr;
    } else {
        if (m_CurrentScreen != nullptr) {
            m_CurrentScreen->Update(delta);
        }
    }

    while (!m_UpdateQueue.empty()) {
        m_UpdateQueue.front()();
        m_UpdateQueue.erase(m_UpdateQueue.begin());
    }
}

void Manager::Draw(double delta)
{
    if (m_CurrentScreen != nullptr) {
        m_CurrentScreen->Draw(delta);
    }

    while (!m_DrawQueue.empty()) {
        m_DrawQueue.front()();
        m_DrawQueue.erase(m_DrawQueue.begin());
    }
}

void Manager::Input(double delta)
{
    if (m_CurrentScreen != nullptr) {
        m_CurrentScreen->Input(delta);
    }

    while (!m_InputQueue.empty()) {
        m_InputQueue.front()();
        m_InputQueue.erase(m_InputQueue.begin());
    }
}

void Manager::FixedUpdate(double fixedDelta)
{
    if (m_CurrentScreen != nullptr) {
        m_CurrentScreen->FixedUpdate(fixedDelta);
    }

    while (!m_FixedUpdateQueue.empty()) {
        m_FixedUpdateQueue.front()();
        m_FixedUpdateQueue.erase(m_FixedUpdateQueue.begin());
    }
}

void Manager::OnKeyDown(const Inputs::State &state)
{
    if (m_CurrentScreen != nullptr) {
        m_CurrentScreen->OnKeyDown(state);
    }
}

void Manager::OnKeyUp(const Inputs::State &state)
{
    if (m_CurrentScreen != nullptr) {
        m_CurrentScreen->OnKeyUp(state);
    }
}

void Manager::AddScreen(uint32_t Id, Base *screen)
{
    m_Screens[Id] = std::unique_ptr<Base>(screen);
}

void Manager::SetScreen(uint32_t Id)
{
    if (m_Screens.find(Id) == m_Screens.end()) {
        throw Exceptions::EstException("Screen not found");
    }

    m_CurrentScreen = m_Screens[Id].get();
    m_CurrentScreen->Attach();
}

void Manager::Enqueue(EnqueueType type, std::function<void()> func)
{
    switch (type) {
        case EnqueueType::Update:
            m_UpdateQueue.push_back(func);
            break;

        case EnqueueType::Draw:
            m_DrawQueue.push_back(func);
            break;

        case EnqueueType::Input:
            m_InputQueue.push_back(func);
            break;

        case EnqueueType::FixedUpdate:
            m_FixedUpdateQueue.push_back(func);
            break;
    }
}

Manager *Manager::Get()
{
    if (instance == nullptr) {
        instance = new Manager();
    }
    return instance;
}

void Manager::Destroy()
{
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
}