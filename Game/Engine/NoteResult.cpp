#include "NoteResult.hpp"
#include <iostream>
#include "RhythmEngine.hpp"

std::tuple<bool, NoteResult> TimeToResult(RhythmEngine* engine, double time) {
	auto timings = engine->GetTimingWindow();
	time = std::abs(time);

	if (time <= timings[0]) {
		return { true, NoteResult::COOL };
	}
	else if (time <= timings[1]) {
		return { true, NoteResult::GOOD };
	}
	else if (time <= timings[2]) {
		return { true, NoteResult::BAD };
	}
	else if (time <= timings[3]) {
		return { true, NoteResult::MISS };
	}

	return { false, NoteResult::MISS };
}
