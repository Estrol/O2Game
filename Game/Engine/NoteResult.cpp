#include "NoteResult.hpp"
#include <iostream>
#include <iomanip>
#include "RhythmEngine.hpp"

std::tuple<bool, NoteResult> TimeToResult(RhythmEngine* engine, double noteTime, double time) {
	double beatDiff = std::abs(time);
	if (beatDiff <= kNoteCoolHitWindowMax) {
		return { true, NoteResult::COOL };
	}
	else if (beatDiff <= kNoteGoodHitWindowMax) {
		return { true, NoteResult::GOOD };
	}
	else if (beatDiff <= kNoteBadHitWindowMax) {
		return { true, NoteResult::BAD };
	}
	else if (beatDiff <= kNoteEarlyMissWindowMin) {
		return { true, NoteResult::MISS };
	}

	return { false, NoteResult::MISS };
}
