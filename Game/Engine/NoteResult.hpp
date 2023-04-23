#pragma once
#include <tuple>

class RhythmEngine;
class Note;

enum class NoteResult {
	MISS,
	BAD,
	GOOD,
	COOL
};

enum class HoldResult {
	HoldBreak,
	HoldAdd
};

//namespace {
//	const double kNoteCoolHitWindowMax = 0.2;
//	const double kNoteGoodHitWindowMax = 0.5;
//	const double kNoteBadHitWindowMax = 0.8;
//	const double kNoteEarlyMissWindowMin = 0.85;
//}

namespace {
	const double kNoteCoolHitWindowMax = 25;
	const double kNoteGoodHitWindowMax = 97;
	const double kNoteBadHitWindowMax = 150;
	const double kNoteEarlyMissWindowMin = 200;

	const double kNoteCoolHitRatio = 0.2;
	const double kNoteGoodHitRatio = 0.5;
	const double kNoteBadHitRatio = 0.8;
	const double kNoteEarlyMissRatio = 0.85;
}

bool IsMissed(RhythmEngine* engine, Note* note);
bool IsAccept(RhythmEngine* engine, Note* note);
std::tuple<bool, NoteResult> TimeToResult(RhythmEngine* engine, Note* note);