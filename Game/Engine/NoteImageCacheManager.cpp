#include "NoteImageCacheManager.hpp"
#include <algorithm>

constexpr int MAX_OBJECTS = 50;

NoteImageCacheManager::NoteImageCacheManager() {
	m_noteTextures = std::unordered_map<NoteImageType, std::vector<DrawableNote*>>();
}

NoteImageCacheManager::~NoteImageCacheManager() {
	for (auto& it : m_noteTextures) {
		for (auto& it2 : it.second) {
			delete it2;
		}
	}

	for (auto& it : m_tileTextures) {
		for (auto& it2 : it.second) {
			delete it2;
		}
	}
}

NoteImageCacheManager* NoteImageCacheManager::s_instance = nullptr;

void NoteImageCacheManager::Repool(DrawableNote* image, NoteImageType noteType) {
	if (image == nullptr) return;

	auto& it = m_noteTextures[noteType];

	if (it.size() >= MAX_OBJECTS) {
		delete image;
		return;
	}

	it.push_back(image);
}

void NoteImageCacheManager::RepoolTile(DrawableTile* image, NoteImageType noteType) {
	if (image == nullptr) return;

	auto& it = m_tileTextures[noteType];

	if (it.size() >= MAX_OBJECTS) {
		delete image;
		return;
	}

	it.push_back(image);
}

DrawableNote* NoteImageCacheManager::Depool(NoteImageType noteType) {
	if (noteType >= NoteImageType::WHITE && noteType <= NoteImageType::YELLOW) {
		auto& it = m_noteTextures[noteType];
		DrawableNote* image = nullptr;
		if (it.size() > 0) {
			image = it.back();
			it.pop_back();
		}
		else {
			image = new DrawableNote(GameNoteResource::GetNoteTexture(noteType));
		}

		return image;
	}
	else {
		return nullptr;
	}
}

DrawableTile* NoteImageCacheManager::DepoolTile(NoteImageType noteType) {
	if (noteType >= NoteImageType::HOLD_WHITE && noteType <= NoteImageType::HOLD_YELLOW) {
		auto& it = m_tileTextures[noteType];
		DrawableTile* image = nullptr;
		if (it.size() > 0) {
			image = it.back();
			it.pop_back();
		}
		else {
			image = new DrawableTile(GameNoteResource::GetNoteTexture(noteType));
		}

		return image;
	}
	else {
		return nullptr;
	}
}

NoteImageCacheManager* NoteImageCacheManager::GetInstance() {
	if (s_instance == nullptr) {
		s_instance = new NoteImageCacheManager();
	}

	return s_instance;
}

void NoteImageCacheManager::Release() {
	if (s_instance) {
		delete s_instance;
		s_instance = nullptr;
	}
}
