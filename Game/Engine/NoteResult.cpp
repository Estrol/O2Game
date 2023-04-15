#include "NoteResult.hpp"
#include <iostream>
#include <iomanip>
#include "RhythmEngine.hpp"

std::tuple<bool, NoteResult> TimeToResult(RhythmEngine* engine, double noteTime, double time) {
	time /= engine->GetSongRate();
	
	auto timings = engine->GetTimingWindow();

	double beatDiff = std::abs(time);
	if (beatDiff <= timings[0]) {
		return { true, NoteResult::COOL };
	}
	else if (beatDiff <= timings[1]) {
		return { true, NoteResult::GOOD };
	}
	else if (beatDiff <= timings[2]) {
		return { true, NoteResult::BAD };
	}
	else if (beatDiff <= timings[3]) {
		return { true, NoteResult::MISS };
	}

	return { false, NoteResult::MISS };
}
