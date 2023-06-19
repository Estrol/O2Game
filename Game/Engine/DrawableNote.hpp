#pragma once
#include "../../Engine/EstEngine.hpp"
#include "FrameTimer.hpp"

struct NoteImage;

class DrawableNote : public FrameTimer {
public:
	DrawableNote(NoteImage* frame);
};