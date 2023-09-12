#include "BeatBasedJudge.h"
#include "../../EnvironmentSetup.hpp"

#include "../RhythmEngine.hpp"
#include "../Note.hpp"

BeatBasedJudge::BeatBasedJudge(RhythmEngine* engine) : JudgeBase(engine) {}

namespace {
	const double kNoteCoolHitRatio = 6.0;
	const double kNoteGoodHitRatio = 18.0;
	const double kNoteBadHitRatio = 25.0;
	const double kNoteEarlyMissRatio = 30.0;

    const double kBaseBPM = 240.0;
    const double kMaxTicks = 192.0;
}

std::tuple<bool, NoteResult> BeatBasedJudge::CalculateResult(Note* note) {
    double audioPos = m_engine->GetGameAudioPosition();
	double hitTime = note->GetHitTime();
    int difficulty = EnvironmentSetup::GetInt("Difficulty");

    double beat = kBaseBPM / kMaxTicks / note->GetBPMTime() * 1000.0;
    double cool = beat * kNoteCoolHitRatio;
    double good = beat * kNoteGoodHitRatio;
    double bad = beat * kNoteBadHitRatio;
    double miss = beat * kNoteEarlyMissRatio;

    double time = abs(hitTime - audioPos);

    if (time <= cool) {
        return { true, NoteResult::COOL };
    }
    else if (time <= good) {
        return { true, NoteResult::GOOD };
    }
    else if (time <= bad) {
        return { true, NoteResult::BAD };
    }
    else if (time <= miss) {
        return { true, NoteResult::MISS };
    }

    return { false, NoteResult::MISS };
}

bool BeatBasedJudge::IsAccepted(Note* note) {
    double audioPos = m_engine->GetGameAudioPosition();
	double hitTime = note->GetHitTime();
    double bad = kBaseBPM / kMaxTicks / note->GetBPMTime() * 1000.0 * kNoteBadHitRatio;

    return hitTime - audioPos <= bad;
}

bool BeatBasedJudge::IsMissed(Note* note) {
    int difficulty = EnvironmentSetup::GetInt("Difficulty");
    double audioPos = m_engine->GetGameAudioPosition();
    double hitTime = note->GetHitTime();
    double miss = kBaseBPM / kMaxTicks / note->GetBPMTime() * 1000.0 * kNoteEarlyMissRatio;

    return hitTime - audioPos < -miss;
}