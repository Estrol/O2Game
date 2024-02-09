/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "NoteResult.h"

struct JudgeTime
{
    float cool = 0.0f;
    float good = 0.0f;
    float bad = 0.0f;
    float miss = 0.0f;
};

class JudgeBase
{
public:
    JudgeBase(RhythmEngine *engine);
    virtual std::tuple<bool, NoteResult>   CalculateResult(Note *note, double time);
    virtual std::tuple<int, int, int, int> GetJudgeTime();

    virtual bool IsMissed(Note *note, double time);
    virtual bool IsAccepted(Note *note, double time);

protected:
    RhythmEngine *m_engine;
};