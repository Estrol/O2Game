#include "NoteResult.hpp"
#include <iostream>

namespace {
	const double kNoteCoolHitWindowMax = 25;
	const double kNoteGoodHitWindowMax = 97;
	const double kNoteBadHitWindowMax = 150;
	const double kNoteEarlyMissWindowMin = 200;
}

std::tuple<bool, NoteResult> TimeToResult(double time) {
	time = std::abs(time);

	if (time <= kNoteCoolHitWindowMax) {
		return { true, NoteResult::COOL };
	}
	else if (time <= kNoteGoodHitWindowMax) {
		return { true, NoteResult::GOOD };
	}
	else if (time <= kNoteBadHitWindowMax) {
		return { true, NoteResult::BAD };
	}
	else if (time <= kNoteEarlyMissWindowMin) {
		return { true, NoteResult::MISS };
	}

	return { false, NoteResult::MISS };
}
