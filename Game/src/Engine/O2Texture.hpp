#pragma once
#include "Texture/Texture2D.h"
#include "../Resources/GameResources.hpp"

class O2Texture : public Texture2D {
public:
	O2Texture(OJSFrame* frame);

protected:
	void LoadImageResources(Boundary* position);
};