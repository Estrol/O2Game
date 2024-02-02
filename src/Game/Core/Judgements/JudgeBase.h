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
    virtual std::tuple<bool, NoteResult>   CalculateResult(Note *note);
    virtual std::tuple<int, int, int, int> GetJudgeTime();

    virtual bool IsMissed(Note *note);
    virtual bool IsAccepted(Note *note);

protected:
    RhythmEngine *m_engine;
};