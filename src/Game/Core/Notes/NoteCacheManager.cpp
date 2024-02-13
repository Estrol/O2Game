/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../Resources/NoteImages.h"
#include "NoteCacheManager.h"
#include <Logs.h>

NoteCacheManager::NoteCacheManager()
{
    m_noteTextures = std::unordered_map<NoteImageType, std::vector<Sprite *>>();
}

NoteCacheManager::~NoteCacheManager()
{
    for (auto &it : m_noteTextures) {
        for (auto &it2 : it.second) {
            delete it2;
        }
    }

    for (auto &it : m_holdTextures) {
        for (auto &it2 : it.second) {
            delete it2;
        }
    }

    for (auto &it : m_trailTextures) {
        for (auto &it2 : it.second) {
            delete it2;
        }
    }
}

NoteCacheManager *NoteCacheManager::s_instance = nullptr;

void NoteCacheManager::Repool(Sprite *image, NoteImageType noteType)
{
    if (image == nullptr)
        return;

    auto &it = m_noteTextures[noteType];

    it.push_back(image);
}

void NoteCacheManager::RepoolHold(Sprite *image, NoteImageType noteType)
{
    if (image == nullptr)
        return;

    auto &it = m_holdTextures[noteType];

    it.push_back(image);
}

void NoteCacheManager::RepoolTrail(Sprite *image, NoteImageType noteType)
{
    if (image == nullptr)
        return;

    auto &it = m_trailTextures[noteType];

    it.push_back(image);
}

Sprite *NoteCacheManager::Depool(NoteImageType noteType)
{
    if (noteType >= NoteImageType::LANE_1 && noteType <= NoteImageType::LANE_7) {
        auto   &it = m_noteTextures[noteType];
        Sprite *image = nullptr;
        if (it.size() > 0) {
            image = it.back();
            it.pop_back();
        } else {
            auto textures = Resources::NoteImages::Get(noteType);

            image = new Sprite(textures->Texture, textures->TexCoords, textures->FrameRate);
            image->Size = UDim2::fromOffset(textures->ImagesRect.Width, textures->ImagesRect.Height);
            image->Repeat = true;
        }

        return image;
    } else {
        return nullptr;
    }
}

Sprite *NoteCacheManager::DepoolHold(NoteImageType noteType)
{
    if (noteType >= NoteImageType::HOLD_LANE_1 && noteType <= NoteImageType::HOLD_LANE_7) {
        auto   &it = m_holdTextures[noteType];
        Sprite *image = nullptr;
        if (it.size() > 0) {
            image = it.back();
            it.pop_back();
        } else {
            auto textures = Resources::NoteImages::Get(noteType);

            image = new Sprite(textures->Texture, textures->TexCoords, textures->FrameRate);
            image->Size = UDim2::fromOffset(textures->ImagesRect.Width, textures->ImagesRect.Height);
            image->Repeat = true;
        }

        return image;
    } else {
        return nullptr;
    }
}

Sprite *NoteCacheManager::DepoolTrail(NoteImageType noteType)
{
    if (noteType >= NoteImageType::TRAIL_UP && noteType <= NoteImageType::TRAIL_DOWN) {
        auto   &it = m_trailTextures[noteType];
        Sprite *image = nullptr;
        if (it.size() > 0) {
            image = it.back();
            it.pop_back();
        } else {
            auto textures = Resources::NoteImages::Get(noteType);

            image = new Sprite(textures->Texture, textures->TexCoords, textures->FrameRate);
            image->Size = UDim2::fromOffset(textures->ImagesRect.Width, textures->ImagesRect.Height);
            image->Repeat = true;
        }

        return image;
    } else {
        return nullptr;
    }
}

NoteCacheManager *NoteCacheManager::Get()
{
    if (s_instance == nullptr) {
        s_instance = new NoteCacheManager();
    }

    return s_instance;
}

void NoteCacheManager::Release()
{
    if (s_instance) {
        int holdTotal = 0;
        for (auto &it : s_instance->m_holdTextures) {
            holdTotal += it.second.size();
        }

        int noteTotal = 0;
        for (auto &it : s_instance->m_noteTextures) {
            noteTotal += it.second.size();
        }

        int trailTotal = 0;
        for (auto &it : s_instance->m_trailTextures) {
            trailTotal += it.second.size();
        }

        int total = holdTotal + noteTotal + trailTotal;

        Logs::Puts(
            "[NoteCacheManager] Release about: %d hold, %d note, %d trail, total: %d",
            holdTotal,
            noteTotal,
            trailTotal,
            total);

        delete s_instance;
        s_instance = nullptr;
    }
}
