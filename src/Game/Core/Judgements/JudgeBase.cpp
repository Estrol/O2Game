/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "JudgeBase.h"

JudgeBase::JudgeBase(RhythmEngine *engine)
{
    m_engine = engine;
}

std::tuple<bool, NoteResult> JudgeBase::CalculateResult(Note *note)
{
    return { false, NoteResult::MISS };
}

std::tuple<int, int, int, int> JudgeBase::GetJudgeTime()
{
    return { 0, 0, 0, 0 };
}

bool JudgeBase::IsMissed(Note *note)
{
    return false;
}

bool JudgeBase::IsAccepted(Note *note)
{
    return false;
}