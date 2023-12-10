#pragma once
#include "JudgeBase.h"

class BeatBasedJudge : public JudgeBase
{
public:
    BeatBasedJudge(RhythmEngine *engine);

    std::tuple<bool, NoteResult> CalculateResult(Note *note) override;
    bool                         IsMissed(Note *note) override;
    bool                         IsAccepted(Note *note) override;
};