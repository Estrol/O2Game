#pragma once
#include <vector>

#include "../Resources/GameResources.hpp"
#include "Texture/Texture2D.h"

class O2Texture : public Texture2D
{
public:
    O2Texture();
    O2Texture(OJSFrame *frame);

    // static std::vector<O2Texture> Load(O2ResourceType opi, std::string ojs);

protected:
    void LoadImageResources(Boundary *position);
};