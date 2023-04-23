#include "NoteResult.hpp"
#include <iostream>
#include <iomanip>
#include "RhythmEngine.hpp"

float CalculateBeatDiff(RhythmEngine* engine, Note* note) {
	double audioPos = engine->GetGameAudioPosition();
	double hitTime = note->GetHitTime();
	double bpm = note->GetBPMTime();

	return (audioPos - hitTime) * bpm / 60000.0;
}

bool IsMissed(RhythmEngine* engine, Note* note) {
	return CalculateBeatDiff(engine, note) > kNoteBadHitRatio;
}

bool IsAccept(RhythmEngine* engine, Note* note) {
	return CalculateBeatDiff(engine, note) >= kNoteBadHitRatio;
}

std::tuple<bool, NoteResult> TimeToResult(RhythmEngine* engine, Note* note) {
	double beatDiff = std::abs(CalculateBeatDiff(engine, note));

	if (beatDiff <= kNoteCoolHitRatio) {
		return { true, NoteResult::COOL };
	}
	else if (beatDiff <= kNoteGoodHitRatio) {
		return { true, NoteResult::GOOD };
	}
	else if (beatDiff <= kNoteBadHitRatio) {
		return { true, NoteResult::BAD };
	}
	else if (beatDiff <= kNoteEarlyMissRatio) {
		return { true, NoteResult::MISS };
	}

	return { false, NoteResult::MISS }; 
}
