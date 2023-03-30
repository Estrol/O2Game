#pragma once
#include "../../Engine/EstEngine.hpp"

struct NoteImage;

class DrawableTile : public Tile2D {
public:
	DrawableTile(NoteImage* frame);

protected:
	void LoadResources(RECT& rect);
};