#pragma once
#include <iostream>
#include "DrawableNote.hpp"
#include "DrawableTile.hpp"

#include "../Resources/GameResources.hpp"

class NoteImageCacheManager {
public:
	NoteImageCacheManager();
	~NoteImageCacheManager();

	// Store a note texture in the cache
	void Repool(DrawableNote* image, NoteImageType noteType);
	void RepoolTile(DrawableTile* image, NoteImageType noteType);

	// Get a note texture from the cache if exists, otherwise create a new one
	DrawableNote* Depool(NoteImageType noteType);
	DrawableTile* DepoolTile(NoteImageType noteType);

	static NoteImageCacheManager* GetInstance();
	static void Release();

private:
	static NoteImageCacheManager* s_instance;

	std::unordered_map<NoteImageType, std::vector<DrawableNote*>> m_noteTextures;
	std::unordered_map<NoteImageType, std::vector<DrawableTile*>> m_tileTextures;
};