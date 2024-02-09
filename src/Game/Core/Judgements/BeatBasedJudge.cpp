/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../../Env.h"
#include "../Notes/Note.h"
#include "../RhythmEngine.h"
#include "BeatBasedJudge.h"

BeatBasedJudge::BeatBasedJudge(RhythmEngine *engine) : JudgeBase(engine) {}

namespace {
    const double kNoteCoolHitRatio = 6.0;
    const double kNoteGoodHitRatio = 18.0;
    const double kNoteBadHitRatio = 25.0;

    const double kHitBaseBPS = 240.0 * 1000.0;
    const double kHitMaxTicks = 192.0;
} // namespace

std::tuple<bool, NoteResult> BeatBasedJudge::CalculateResult(Note *note, double time)
{
    double hitTime = note->GetHitTime();

    double hitOffset = abs(hitTime - time) / (kHitBaseBPS / note->GetBPMTime()) * kHitMaxTicks;

    if (hitOffset <= kNoteCoolHitRatio)
        return { true, NoteResult::COOL };
    else if (hitOffset <= kNoteGoodHitRatio)
        return { true, NoteResult::GOOD };
    else if (hitOffset <= kNoteBadHitRatio)
        return { true, NoteResult::BAD };

    return { false, NoteResult::MISS };
}

bool BeatBasedJudge::IsAccepted(Note *note, double time)
{
    double hitTime = note->GetHitTime();

    double hitOffset = (hitTime - time) / (kHitBaseBPS / note->GetBPMTime()) * kHitMaxTicks;

    return hitOffset <= kNoteBadHitRatio;
}

bool BeatBasedJudge::IsMissed(Note *note, double time)
{
    double hitTime = note->GetHitTime();

    double hitOffset = (hitTime - time) / (kHitBaseBPS / note->GetBPMTime()) * kHitMaxTicks;

    return hitOffset < -kNoteBadHitRatio;
}