/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

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