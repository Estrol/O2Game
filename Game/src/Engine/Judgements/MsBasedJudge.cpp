#include "MsBasedJudge.h"

#include "../../EnvironmentSetup.hpp"
#include "../RhythmEngine.hpp"
#include "../Note.hpp"

namespace {
    const double kNoteCoolHitRatio = 173;
	const double kNoteGoodHitRatio = 125;
	const double kNoteBadHitRatio = 41;
	const double kNoteEarlyMissRatio = 184;
}

MsBasedJudge::MsBasedJudge(RhythmEngine* engine) : JudgeBase(engine) {

}

std::tuple<bool, NoteResult> MsBasedJudge::CalculateResult(Note* note) {
    double audioPos = m_engine->GetGameAudioPosition();
    double noteTime = note->GetHitTime();

    double diff = std::abs(audioPos - noteTime);

    int difficulty = EnvironmentSetup::GetInt("Difficulty");
    float decrement = 0.0f;
    switch (difficulty) {
        case 1: {
            decrement = 0.1f;
            break;
        }

        case 2: {
            decrement = 0.15f;
            break;
        }
    }

    // (BASE - (BASE * decrement))
    if (diff <= kNoteCoolHitRatio - (kNoteCoolHitRatio * decrement)) {
        return { true, NoteResult::COOL };
    }
    else if (diff <= kNoteGoodHitRatio - (kNoteGoodHitRatio * decrement)) {
        return { true, NoteResult::GOOD };
    }
    else if (diff <= kNoteBadHitRatio - (kNoteBadHitRatio * decrement)) {
        return { true, NoteResult::BAD };
    }
    else if (diff <= kNoteEarlyMissRatio - (kNoteEarlyMissRatio * decrement)) {
        return { true, NoteResult::MISS };
    }

    return { false, NoteResult::MISS };
}

bool MsBasedJudge::IsAccepted(Note* note) {
    double audioPos = m_engine->GetGameAudioPosition();
    double noteTime = note->GetHitTime();

    double diff = std::abs(audioPos - noteTime);

    return diff <= kNoteBadHitRatio;
}

bool MsBasedJudge::IsMissed(Note* note) {
    double audioPos = m_engine->GetGameAudioPosition();
    double noteTime = note->GetHitTime();

    double diff = std::abs(audioPos - noteTime);

    return diff > kNoteEarlyMissRatio;
}