/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "../Drawable/Sprite.h"
#include "../Enums/NoteImageType.h"
#include <unordered_map>

class NoteCacheManager
{
public:
    NoteCacheManager();
    ~NoteCacheManager();

    // Store a note texture in the cache
    void Repool(Sprite *image, NoteImageType noteType);
    void RepoolHold(Sprite *image, NoteImageType noteType);
    void RepoolTrail(Sprite *image, NoteImageType noteType);

    // Get a note texture from the cache if exists, otherwise create a new one
    Sprite *Depool(NoteImageType noteType);
    Sprite *DepoolHold(NoteImageType noteType);
    Sprite *DepoolTrail(NoteImageType noteType);

    static NoteCacheManager *Get();
    static void              Release();

private:
    static NoteCacheManager *s_instance;

    std::unordered_map<NoteImageType, std::vector<Sprite *>> m_noteTextures;
    std::unordered_map<NoteImageType, std::vector<Sprite *>> m_holdTextures;
    std::unordered_map<NoteImageType, std::vector<Sprite *>> m_trailTextures;
};