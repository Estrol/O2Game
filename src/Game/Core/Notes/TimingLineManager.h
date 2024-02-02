/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

#include "../../Data/iterate_queue.h"
#include "TimingLine.h"
#include <vector>

class RhythmEngine;

class TimingLineManager
{
public:
    TimingLineManager(RhythmEngine *engine);
    TimingLineManager(RhythmEngine *engine, std::vector<double> list);
    ~TimingLineManager();

    void Init();
    void Update(double delta);
    void Render(double delta);

private:
    RhythmEngine                               *m_engine;
    iterable_queue<TimingLineDesc>              m_timingInfos;
    iterable_queue<std::shared_ptr<TimingLine>> m_timingLines;
};