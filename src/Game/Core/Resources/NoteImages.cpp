/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../Skinning/LuaSkin.h"
#include "NoteImages.h"
#include <Exceptions/EstException.h>
#include <Graphics/Renderer.h>
#include <Misc/Filesystem.h>
#include <filesystem>
#include <fstream>
#include <unordered_map>

using namespace Resources;

namespace {
    bool                                                          s_Loaded = false;
    std::unordered_map<NoteImageType, std::unique_ptr<NoteImage>> s_NoteImages;
} // namespace

void NoteImages::LoadImageResources()
{
    if (s_Loaded) {
        throw Exceptions::EstException("Note images already loaded");
    }

    auto manager = LuaSkin::Get();
    auto renderer = Graphics::Renderer::Get();

    manager->LoadScript(SkinGroup::Notes);

    for (int i = 0; i < 7; i++) {
        NoteValue note = manager->GetNote("LaneHit" + std::to_string(i));
        NoteValue hold = manager->GetNote("LaneHold" + std::to_string(i));

        auto noteImage = std::make_unique<NoteImage>();
        auto holdImage = std::make_unique<NoteImage>();

        noteImage->MaxFrames = (int)note.Files.size();
        holdImage->MaxFrames = (int)hold.Files.size();

        noteImage->Images.resize(noteImage->MaxFrames);
        holdImage->Images.resize(holdImage->MaxFrames);

        Rect noteSize = { 0, 0, (int)note.Size.X.Offset, (int)note.Size.Y.Offset };
        Rect holdSize = { 0, 0, (int)hold.Size.X.Offset, (int)hold.Size.Y.Offset };

        for (int j = 0; j < noteImage->MaxFrames; j++) {
            auto path = note.Files[j];
            if (!std::filesystem::exists(path)) {
                throw Exceptions::EstException("File: %s is not found!", path.c_str());
            }

            auto texture = renderer->LoadTexture(path);
            // Rect size = texture->GetSize();

            noteImage->Images[j] = new Image(texture);
            noteImage->ImagesRect = noteSize;
        }

        for (int j = 0; j < holdImage->MaxFrames; j++) {
            auto path = hold.Files[j];
            if (!std::filesystem::exists(path)) {
                throw Exceptions::EstException("File: %s is not found!", path.c_str());
            }

            auto texture = renderer->LoadTexture(path);
            // Rect size = texture->GetSize();

            holdImage->Images[j] = new Image(texture);
            holdImage->ImagesRect = holdSize;
        }

        s_NoteImages[(NoteImageType)i] = std::move(noteImage);
        s_NoteImages[(NoteImageType)(i + 7)] = std::move(holdImage);
    }

    NoteValue trailUp = manager->GetNote("NoteTrailUp");
    NoteValue trailDown = manager->GetNote("NoteTrailDown");

    auto trailUpImg = std::make_unique<NoteImage>();
    auto trailDownImg = std::make_unique<NoteImage>();

    int trailUpMaxFrames = (int)trailUp.Files.size();
    int trailDownMaxFrames = (int)trailDown.Files.size();

    trailUpImg->Images.resize(trailUpMaxFrames);
    trailDownImg->Images.resize(trailDownMaxFrames);

    Rect trailUpSize = { 0, 0, (int)trailUp.Size.X.Offset, (int)trailUp.Size.Y.Offset };
    Rect trailDownSize = { 0, 0, (int)trailDown.Size.X.Offset, (int)trailDown.Size.Y.Offset };

    for (int i = 0; i < trailUpMaxFrames; i++) {
        auto path = trailUp.Files[i];
        if (!std::filesystem::exists(path)) {
            throw Exceptions::EstException("File: %s is not found!", path.c_str());
        }

        auto texture = renderer->LoadTexture(path);
        // Rect size = texture->GetSize();

        trailUpImg->Images[i] = new Image(texture);
        trailUpImg->ImagesRect = trailDownSize;
    }

    for (int i = 0; i < trailDownMaxFrames; i++) {
        auto path = trailDown.Files[i];
        if (!std::filesystem::exists(path)) {
            throw Exceptions::EstException("File: %s is not found!", path.c_str());
        }

        auto texture = renderer->LoadTexture(path);
        Rect size = texture->GetSize();

        trailDownImg->Images[i] = new Image(texture);
        trailDownImg->ImagesRect = trailDownSize;
    }

    s_NoteImages[NoteImageType::TRAIL_UP] = std::move(trailUpImg);
    s_NoteImages[NoteImageType::TRAIL_DOWN] = std::move(trailDownImg);

    s_Loaded = true;
}

void NoteImages::UnloadImageResources()
{
    for (auto &noteImage : s_NoteImages) {
        for (auto &image : noteImage.second->Images) {
            delete image;
        }
    }

    s_NoteImages.clear();
    s_Loaded = false;
}

NoteImage *NoteImages::Get(NoteImageType type)
{
    if (!s_Loaded) {
        throw Exceptions::EstException("Note images not loaded");
    }

    return s_NoteImages[type].get();
}