#include "NoteImageCacheManager.hpp"
#include <algorithm>
#include <Logs.h>

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

	for (auto& it : m_holdTextures) {
		for (auto& it2 : it.second) {
			delete it2;
		}
	}

	for (auto& it : m_trailTextures) {
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

	it.emplace_back(image);
}

void NoteImageCacheManager::RepoolHold(DrawableNote* image, NoteImageType noteType) {
	if (image == nullptr) return;

	auto& it = m_holdTextures[noteType];

	if (it.size() >= MAX_OBJECTS) {
		delete image;
		return;
	}

	it.emplace_back(image);
}

void NoteImageCacheManager::RepoolTrail(DrawableNote* image, NoteImageType noteType) {
	if (image == nullptr) return;

	auto& it = m_trailTextures[noteType];

	if (it.size() >= MAX_OBJECTS) {
		delete image;
		return;
	}

	it.emplace_back(image);
}

DrawableNote* NoteImageCacheManager::Depool(NoteImageType noteType) {
	if (noteType >= NoteImageType::LANE_1 && noteType <= NoteImageType::LANE_7) {
		auto& it = m_noteTextures[noteType];
		DrawableNote* image = nullptr;
		if (it.size() > 0) {
			image = it.back();
			it.pop_back();
		}
		else {
			image = new DrawableNote(GameNoteResource::GetNoteTexture(noteType));
			image->Repeat = true;
		}

		return image;
	}
	else {
		return nullptr;
	}
}

DrawableNote* NoteImageCacheManager::DepoolHold(NoteImageType noteType) {
	if (noteType >= NoteImageType::HOLD_LANE_1 && noteType <= NoteImageType::HOLD_LANE_7) {
		auto& it = m_holdTextures[noteType];
		DrawableNote* image = nullptr;
		if (it.size() > 0) {
			image = it.back();
			it.pop_back();
		}
		else {
			image = new DrawableNote(GameNoteResource::GetNoteTexture(noteType));
			image->Repeat = true;
		}

		return image;
	}
	else {
		return nullptr;
	}
}

DrawableNote* NoteImageCacheManager::DepoolTrail(NoteImageType noteType) {
	if (noteType >= NoteImageType::TRAIL_UP && noteType <= NoteImageType::TRAIL_DOWN) {
		auto& it = m_trailTextures[noteType];
		DrawableNote* image = nullptr;
		if (it.size() > 0) {
			image = it.back();
			it.pop_back();
		}
		else {
			image = new DrawableNote(GameNoteResource::GetNoteTexture(noteType));
			image->Repeat = true;
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
		Logs::Puts("[NoteImageCacheManager] Release about: hold=%d, note=%d, trail=%d", s_instance->m_holdTextures.size(), s_instance->m_noteTextures.size(), s_instance->m_trailTextures.size());

		delete s_instance;
		s_instance = nullptr;
	}
}
