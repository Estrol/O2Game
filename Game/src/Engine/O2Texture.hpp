#pragma once
#include <vector>

#include "Texture/Texture2D.h"
#include "../Resources/GameResources.hpp"

class O2Texture : public Texture2D {
public:
	O2Texture();
	O2Texture(OJSFrame* frame);
	
	//static std::vector<O2Texture> Load(O2ResourceType opi, std::string ojs);

protected:
	void LoadImageResources(Boundary* position);
};