#pragma once
#include "DrawableNote.hpp"
#include "DrawableTile.hpp"
#include <iostream>

#include "../Resources/GameResources.hpp"

class NoteImageCacheManager
{
public:
    NoteImageCacheManager();
    ~NoteImageCacheManager();

    // Store a note texture in the cache
    void Repool(DrawableNote *image, NoteImageType noteType);
    void RepoolHold(DrawableNote *image, NoteImageType noteType);
    void RepoolTrail(DrawableNote *image, NoteImageType noteType);

    // Get a note texture from the cache if exists, otherwise create a new one
    DrawableNote *Depool(NoteImageType noteType);
    DrawableNote *DepoolHold(NoteImageType noteType);
    DrawableNote *DepoolTrail(NoteImageType noteType);

    static NoteImageCacheManager *GetInstance();
    static void                   Release();

private:
    static NoteImageCacheManager *s_instance;

    std::unordered_map<NoteImageType, std::vector<DrawableNote *>> m_noteTextures;
    std::unordered_map<NoteImageType, std::vector<DrawableNote *>> m_holdTextures;
    std::unordered_map<NoteImageType, std::vector<DrawableNote *>> m_trailTextures;
};