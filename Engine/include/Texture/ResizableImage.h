#pragma once

#include "Texture2D.h"

class ResizableImage : public Texture2D
{
public:
    ResizableImage();
    ResizableImage(int X, int Y, char color);
};