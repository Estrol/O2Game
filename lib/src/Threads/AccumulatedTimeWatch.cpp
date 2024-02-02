/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Threads/AccumulatedTimeWatch.h>

AccurateTimeWatch::AccurateTimeWatch(float fixedStep)
    : m_AccumulatedTime(0.0), m_FixedStep(fixedStep)
{
}

AccurateTimeWatch::~AccurateTimeWatch()
{
}

void AccurateTimeWatch::Update(double delta)
{
    m_AccumulatedTime += delta;
}

bool AccurateTimeWatch::Tick()
{
    if (m_AccumulatedTime >= m_FixedStep) {
        m_AccumulatedTime -= m_FixedStep;
        return true;
    }

    return false;
}