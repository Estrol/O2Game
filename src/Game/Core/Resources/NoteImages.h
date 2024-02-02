/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "../Drawable/Image.h"
#include "../Enums/NoteImageType.h"
#include <Graphics/Utils/Rect.h>
#include <vector>

namespace Resources {
    struct NoteImage
    {
        std::vector<Image *> Images;
        Rect                 ImagesRect;
        int                  MaxFrames;
    };

    namespace NoteImages {
        void LoadImageResources();
        void UnloadImageResources();

        NoteImage *Get(NoteImageType type);
    } // namespace NoteImages
} // namespace Resources