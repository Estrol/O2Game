#pragma once
#include <tuple>

class RhythmEngine;

enum class NoteResult {
	MISS,
	BAD,
	GOOD,
	COOL
};

namespace {
	const double kNoteCoolHitWindowMax = 25;
	const double kNoteGoodHitWindowMax = 97;
	const double kNoteBadHitWindowMax = 150;
	const double kNoteEarlyMissWindowMin = 200;
}

std::tuple<bool, NoteResult> TimeToResult(RhythmEngine* engine, double time);