#pragma once
#include <tuple>

enum class NoteResult {
	MISS,
	BAD,
	GOOD,
	COOL
};

std::tuple<bool, NoteResult> TimeToResult(double time);