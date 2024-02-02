/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "NoteImages.h"
#include "../Skinning/SkinManager.h"
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

    auto manager = SkinManager::Get();
    auto renderer = Graphics::Renderer::Get();

    auto skinPath = manager->GetPath();
    auto skinNotePath = skinPath / "Notes";

    for (int i = 0; i < 7; i++) {
        NoteValue note = manager->GetNote(SkinGroup::Notes, "LaneHit" + std::to_string(i));
        NoteValue hold = manager->GetNote(SkinGroup::Notes, "LaneHold" + std::to_string(i));

        auto noteImage = std::make_unique<NoteImage>();
        auto holdImage = std::make_unique<NoteImage>();

        noteImage->MaxFrames = note.numOfFiles;
        holdImage->MaxFrames = hold.numOfFiles;

        noteImage->Images.resize(note.numOfFiles);
        holdImage->Images.resize(hold.numOfFiles);

        for (int j = 0; j < note.numOfFiles; j++) {
            auto path = skinNotePath / (note.fileName + std::to_string(j) + ".png");
            if (!std::filesystem::exists(path)) {
                throw Exceptions::EstException("File: %s is not found!", path.string().c_str());
            }

            auto texture = renderer->LoadTexture(path);
            Rect size = texture->GetSize();

            noteImage->Images[j] = new Image(texture);
            noteImage->ImagesRect = size;
        }

        for (int j = 0; j < hold.numOfFiles; j++) {
            auto path = skinNotePath / (hold.fileName + std::to_string(j) + ".png");
            if (!std::filesystem::exists(path)) {
                throw Exceptions::EstException("File: %s is not found!", path.string().c_str());
            }

            auto texture = renderer->LoadTexture(path);
            Rect size = texture->GetSize();

            holdImage->Images[j] = new Image(texture);
            holdImage->ImagesRect = size;
        }

        s_NoteImages[(NoteImageType)i] = std::move(noteImage);
        s_NoteImages[(NoteImageType)(i + 7)] = std::move(holdImage);
    }

    NoteValue trailUp = manager->GetNote(SkinGroup::Notes, "NoteTrailUp");
    NoteValue trailDown = manager->GetNote(SkinGroup::Notes, "NoteTrailDown");

    auto trailUpImg = std::make_unique<NoteImage>();
    auto trailDownImg = std::make_unique<NoteImage>();

    trailUpImg->Images.resize(trailUp.numOfFiles);
    trailDownImg->Images.resize(trailDown.numOfFiles);

    for (int i = 0; i < trailUp.numOfFiles; i++) {
        auto path = skinNotePath / (trailUp.fileName + std::to_string(i) + ".png");
        if (!std::filesystem::exists(path)) {
            throw Exceptions::EstException("File: %s is not found!", path.string().c_str());
        }

        auto texture = renderer->LoadTexture(path);
        Rect size = texture->GetSize();

        trailUpImg->Images[i] = new Image(texture);
        trailUpImg->ImagesRect = size;
    }

    for (int i = 0; i < trailDown.numOfFiles; i++) {
        auto path = skinNotePath / (trailDown.fileName + std::to_string(i) + ".png");
        if (!std::filesystem::exists(path)) {
            throw Exceptions::EstException("File: %s is not found!", path.string().c_str());
        }

        auto texture = renderer->LoadTexture(path);
        Rect size = texture->GetSize();

        trailDownImg->Images[i] = new Image(texture);
        trailDownImg->ImagesRect = size;
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