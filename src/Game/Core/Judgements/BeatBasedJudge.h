/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "JudgeBase.h"

class BeatBasedJudge : public JudgeBase
{
public:
    BeatBasedJudge(RhythmEngine *engine);

    std::tuple<bool, NoteResult> CalculateResult(Note *note, double time) override;
    bool                         IsMissed(Note *note, double time) override;
    bool                         IsAccepted(Note *note, double time) override;
};