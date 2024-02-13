/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "../Drawable/Sprite.h"
#include "../Enums/NoteImageType.h"
#include <Graphics/Utils/Rect.h>
#include <Math/Color3.h>
#include <vector>

namespace Resources {
    struct NoteImage
    {
        std::shared_ptr<Graphics::Texture2D> Texture;
        std::vector<std::vector<glm::vec2>>  TexCoords;
        Rect                                 ImagesRect;
        int                                  MaxFrames;
        float                                FrameRate;
        Color3                               Color;
    };

    namespace NoteImages {
        void LoadImageResources();
        void UnloadImageResources();

        NoteImage *Get(NoteImageType type);
    } // namespace NoteImages
} // namespace Resources