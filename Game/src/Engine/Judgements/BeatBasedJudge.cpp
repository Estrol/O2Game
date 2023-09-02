#include "BeatBasedJudge.h"
#include "../../EnvironmentSetup.hpp"

#include "../RhythmEngine.hpp"
#include "../Note.hpp"

BeatBasedJudge::BeatBasedJudge(RhythmEngine* engine) : JudgeBase(engine) {}

namespace {
	const double kNoteCoolHitRatio = 0.2;
	const double kNoteGoodHitRatio = 0.5;
	const double kNoteBadHitRatio = 0.8;
	const double kNoteEarlyMissRatio = 0.85;
}

double CalculateBeatDiff(RhythmEngine* engine, JudgeBase* judge, Note* note) {
    // double audioPos = engine->GetGameAudioPosition();
	// double noteTime = note->GetHitTime();
    // double hitTime = noteTime - (noteTime - audioPos);

    // double noteBeat = engine->GetTiming()->GetBeatAt(noteTime);
	// double hitBeat = engine->GetTiming()->GetBeatAt(hitTime);

    // return (noteBeat - hitBeat) / 0.664;

    double audioPos = engine->GetGameAudioPosition();
	double hitTime = note->GetHitTime();
	double bpm = note->GetBPMTime();

	return (audioPos - hitTime) * bpm / 60000.0;
}

std::tuple<bool, NoteResult> BeatBasedJudge::CalculateResult(Note* note) {
    double beatDiff = std::abs(CalculateBeatDiff(m_engine, this, note));

    int difficulty = EnvironmentSetup::GetInt("Difficulty");
    float decrement = 0.0f;
    switch (difficulty) {
        case 1: {
            decrement = 0.025f;
            break;
        }

        case 2: {
            decrement = 0.045f;
            break;
        }
    }

    if (beatDiff <= kNoteCoolHitRatio - decrement) {
        return { true, NoteResult::COOL };
    }
    else if (beatDiff <= kNoteGoodHitRatio - decrement) {
        return { true, NoteResult::GOOD };
    }
    else if (beatDiff <= kNoteBadHitRatio - decrement) {
        return { true, NoteResult::BAD };
    }
    else if (beatDiff <= kNoteEarlyMissRatio - decrement) {
        return { true, NoteResult::MISS };
    }

    return { false, NoteResult::MISS };
}

bool BeatBasedJudge::IsAccepted(Note* note) {
    int difficulty = EnvironmentSetup::GetInt("Difficulty");
    float decrement = 0.0f;
    switch (difficulty) {
        case 1: {
            decrement = 0.025f;
            break;
        }

        case 2: {
            decrement = 0.045f;
            break;
        }
    }

    return CalculateBeatDiff(m_engine, this, note) >= (kNoteBadHitRatio - decrement);
}

bool BeatBasedJudge::IsMissed(Note* note) {
    int difficulty = EnvironmentSetup::GetInt("Difficulty");
    float decrement = 0.0f;
    switch (difficulty) {
        case 1: {
            decrement = 0.025f;
            break;
        }

        case 2: {
            decrement = 0.045f;
            break;
        }
    }

    return CalculateBeatDiff(m_engine, this, note) > (kNoteBadHitRatio - decrement);
}