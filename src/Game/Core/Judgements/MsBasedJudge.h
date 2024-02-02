/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "JudgeBase.h"

class MsBasedJudge : public JudgeBase
{
public:
    MsBasedJudge(RhythmEngine *engine);

    std::tuple<bool, NoteResult> CalculateResult(Note *note) override;
    bool                         IsAccepted(Note *note) override;
    bool                         IsMissed(Note *note) override;
};