#pragma once
#include "../../Engine/EstEngine.hpp"
#include "../Resources/GameResources.hpp"

class O2Texture : public Texture2D {
public:
	O2Texture(OJSFrame* frame);

protected:
	void LoadImageResources(Boundary* position);
};