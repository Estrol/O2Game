/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Threads/TimeWatch.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

TimeWatch::TimeWatch()
{
    m_LastTick = static_cast<double>(SDL_GetTicks64());
    m_CurTick = m_LastTick;
}

TimeWatch::~TimeWatch()
{
}

double TimeWatch::Tick()
{
    if (m_TickRate > 0) {
        double newTick = SDL_GetTicks();
        double deltaTick = 1000.0 / m_TickRate - (newTick - m_LastTick);

        if (deltaTick > 0.0) {
            int delayTicks = static_cast<int>(deltaTick);
            SDL_Delay(delayTicks);
            newTick += delayTicks;
            deltaTick -= delayTicks;
        }

        m_LastTick = (deltaTick < -30.0) ? newTick : newTick + deltaTick;

        double delta = (newTick - m_CurTick) / 1000.0;
        m_CurTick = newTick;

        return delta;
    } else {
        double newTick = static_cast<double>(SDL_GetTicks64());
        double dt = (newTick - m_CurTick) / 1000.0;
        m_CurTick = newTick;

        return dt;
    }
}

void TimeWatch::SetTickRate(double tickRate)
{
    m_TickRate = tickRate;
}