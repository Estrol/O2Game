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