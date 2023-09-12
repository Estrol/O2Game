#include "MsBasedJudge.h"

#include "../../EnvironmentSetup.hpp"
#include "../RhythmEngine.hpp"
#include "../Note.hpp"

namespace {
    const double kNoteCoolHitRatio = 41;
	const double kNoteGoodHitRatio = 125;
	const double kNoteBadHitRatio = 173;
	const double kNoteEarlyMissRatio = 184;
}

MsBasedJudge::MsBasedJudge(RhythmEngine* engine) : JudgeBase(engine) {

}

std::tuple<bool, NoteResult> MsBasedJudge::CalculateResult(Note* note) {
    double audioPos = m_engine->GetGameAudioPosition();
    double noteTime = note->GetHitTime();

    double diff = std::abs(noteTime - audioPos);
    if (diff <= kNoteCoolHitRatio - kNoteCoolHitRatio) {
        return { true, NoteResult::COOL };
    }
    else if (diff <= kNoteGoodHitRatio - kNoteGoodHitRatio) {
        return { true, NoteResult::GOOD };
    }
    else if (diff <= kNoteBadHitRatio - kNoteBadHitRatio) {
        return { true, NoteResult::BAD };
    }
    else if (diff <= kNoteEarlyMissRatio - kNoteEarlyMissRatio) {
        return { true, NoteResult::MISS };
    }

    return { false, NoteResult::MISS };
}

bool MsBasedJudge::IsAccepted(Note* note) {
    double audioPos = m_engine->GetGameAudioPosition();
    double noteTime = note->GetHitTime();

    double diff = noteTime - audioPos;

    return diff <= kNoteBadHitRatio;
}

bool MsBasedJudge::IsMissed(Note* note) {
    double audioPos = m_engine->GetGameAudioPosition();
    double noteTime = note->GetHitTime();

    double diff = noteTime - audioPos;

    return diff < -kNoteEarlyMissRatio;
}