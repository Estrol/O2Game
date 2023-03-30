#include "NoteResult.hpp"

namespace {
	const double kNoteCoolHitWindowMin = -25;
	const double kNoteCoolHitWindowMax = 25;

	const double kNoteGoodHitWindowMin = -97;
	const double kNoteGoodHitWindowMax = 97;

	const double kNoteBadHitWindowMin = -150;
	const double kNoteBadHitWindowMax = 150;

	const double kNoteEarlyMissWindowMin = -200;
}

std::tuple<bool, NoteResult> TimeToResult(double time) {
	if (time >= kNoteBadHitWindowMin && time <= kNoteEarlyMissWindowMin) {
		NoteResult result;

		if (time > kNoteBadHitWindowMin && time < kNoteBadHitWindowMax) {
			result = NoteResult::BAD;
		}
		else if (time > kNoteGoodHitWindowMin && time < kNoteGoodHitWindowMax) {
			result = NoteResult::GOOD;
		}
		else if (time > kNoteCoolHitWindowMin && time < kNoteCoolHitWindowMax) {
			result = NoteResult::COOL;
		}
		else if (time > kNoteCoolHitWindowMax && time < kNoteGoodHitWindowMin) {
			result = NoteResult::GOOD;
		}
		else {
			result = NoteResult::BAD;
		}
		
		return { true, result };
	}

	return { false, NoteResult::MISS };
}
