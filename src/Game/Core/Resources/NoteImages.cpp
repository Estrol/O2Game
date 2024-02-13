/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../Drawable/Sprite.h"
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
        SpriteValue note = manager->GetSprite("LaneHit" + std::to_string(i));
        SpriteValue hold = manager->GetSprite("LaneHold" + std::to_string(i));

        auto noteImage = std::make_unique<NoteImage>();
        auto holdImage = std::make_unique<NoteImage>();

        noteImage->MaxFrames = (int)note.TexCoords.size();
        holdImage->MaxFrames = (int)hold.TexCoords.size();

        noteImage->FrameRate = note.FrameTime;
        holdImage->FrameRate = hold.FrameTime;

        noteImage->Color = note.Color;
        holdImage->Color = hold.Color;

        Rect noteSize = { 0, 0, (int)note.Size.X.Offset, (int)note.Size.Y.Offset };
        Rect holdSize = { 0, 0, (int)hold.Size.X.Offset, (int)hold.Size.Y.Offset };

        if (!std::filesystem::exists(note.Path)) {
            throw Exceptions::EstException("File: %s is not found!", note.Path.c_str());
        }

        std::filesystem::path path = note.Path;

        noteImage->ImagesRect = noteSize;
        noteImage->Texture = renderer->LoadTexture(note.Path);
        noteImage->TexCoords = note.TexCoords;

        if (!std::filesystem::exists(hold.Path)) {
            throw Exceptions::EstException("File: %s is not found!", hold.Path.c_str());
        }

        holdImage->ImagesRect = holdSize;
        holdImage->Texture = renderer->LoadTexture(hold.Path);
        holdImage->TexCoords = hold.TexCoords;

        path = hold.Path;

        s_NoteImages[(NoteImageType)(i)] = std::move(noteImage);
        s_NoteImages[(NoteImageType)(i + 7)] = std::move(holdImage);
    }

    auto trailUp = manager->GetSprite("NoteTrailUp");
    auto trailDown = manager->GetSprite("NoteTrailDown");

    auto trailUpImg = std::make_unique<NoteImage>();
    auto trailDownImg = std::make_unique<NoteImage>();

    int trailUpMaxFrames = (int)trailUp.TexCoords.size();
    int trailDownMaxFrames = (int)trailDown.TexCoords.size();

    trailUpImg->FrameRate = trailUp.FrameTime;
    trailDownImg->FrameRate = trailDown.FrameTime;

    trailUpImg->Color = trailUp.Color;
    trailDownImg->Color = trailDown.Color;

    Rect tailUpSize = { 0, 0, (int)trailUp.Size.X.Offset, (int)trailUp.Size.Y.Offset };
    Rect tailDownSize = { 0, 0, (int)trailDown.Size.X.Offset, (int)trailDown.Size.Y.Offset };

    std::filesystem::path path = trailUp.Path;
    if (!std::filesystem::exists(path)) {
        throw Exceptions::EstException("File: %s is not found!", path.c_str());
    }

    trailUpImg->ImagesRect = tailUpSize;
    trailUpImg->Texture = renderer->LoadTexture(trailUp.Path);
    trailUpImg->TexCoords = trailUp.TexCoords;

    path = trailDown.Path;
    if (!std::filesystem::exists(path)) {
        throw Exceptions::EstException("File: %s is not found!", path.c_str());
    }

    trailDownImg->ImagesRect = tailDownSize;
    trailDownImg->Texture = renderer->LoadTexture(trailDown.Path);
    trailDownImg->TexCoords = trailDown.TexCoords;

    s_NoteImages[NoteImageType::TRAIL_UP] = std::move(trailUpImg);
    s_NoteImages[NoteImageType::TRAIL_DOWN] = std::move(trailDownImg);

    auto measure = manager->GetSprite("MeasureLine");

    auto measureLine = std::make_unique<NoteImage>();
    measureLine->FrameRate = measure.FrameTime;
    measureLine->MaxFrames = (int)measure.TexCoords.size();
    measureLine->Color = measure.Color;

    Rect measureRect = { 0, 0, (int)measure.Size.X.Offset, (int)measure.Size.Y.Offset };
    measureLine->ImagesRect = measureRect;
    measureLine->TexCoords = measure.TexCoords;
    measureLine->Texture = renderer->LoadTexture(measure.Path);

    s_NoteImages[NoteImageType::MEASURE_LINE] = std::move(measureLine);

    s_Loaded = true;
}

void NoteImages::UnloadImageResources()
{
    for (auto &noteImage : s_NoteImages) {
        noteImage.second->Texture.reset();
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