#pragma once
#include "../../Engine/EstEngine.hpp"

struct NoteImage;

class DrawableTile : public Texture2D {
public:
	DrawableTile(NoteImage* frame);
};