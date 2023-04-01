#pragma once
#include "../../Engine/EstEngine.hpp"

struct NoteImage;

class DrawableNote : public Texture2D {
public:
	DrawableNote(NoteImage* frame);
};