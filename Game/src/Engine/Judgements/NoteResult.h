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