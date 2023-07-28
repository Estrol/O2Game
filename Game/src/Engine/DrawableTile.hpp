#pragma once
#include "Texture/Texture2D.h"

struct NoteImage;

class DrawableTile : public Texture2D {
public:
	DrawableTile(NoteImage* frame);
};