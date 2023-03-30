#include "GameTrack.hpp"
#include "RhythmEngine.hpp"
#include "GameAudioSampleCache.hpp"
#include <algorithm>
#include <mutex>

GameTrack::GameTrack(RhythmEngine* engine, int laneIndex, int offset) {
	m_engine = engine;
	m_laneIndex = laneIndex;
	m_laneOffset = offset;
	m_deleteDelay = 0;
}

GameTrack::~GameTrack() {
	for (auto& note : m_notes) {
		delete note;
	}

	for (auto& note : m_noteCaches) {
		delete note;
	}
}

void GameTrack::Update(double delta) {
	for (auto _note = m_notes.begin(); _note != m_notes.end();) {
		auto& note = *_note;

		if (note->IsRemoveable()) {
			m_noteCaches.push_back(note);
			_note = m_notes.erase(_note);
		}
		else {
			if (!note->IsDrawable()) {
				double startTime = note->GetInitialTrackPosition();
				double trackPos = m_engine->GetTrackPosition();

				if (trackPos - startTime > m_engine->GetPrebufferTiming()) {
					note->SetDrawable(true);
				}
			}

			note->Update(delta);
			_note++;
		}
	}
}

void GameTrack::Render(double delta) {
	std::vector<Note*> copy = m_notes;
	for (auto& note : copy) {
		if (note && note->IsDrawable()) {
			note->Render(delta);
		}
	}
}

void GameTrack::OnKeyUp() {
	if (m_callback) {
		m_callback(m_laneIndex, false);
	}

	std::vector<Note*> copy = m_notes;
	for (auto& it : copy) {
		auto result = it->CheckRelease();

		if (std::get<bool>(result)) {
			it->OnRelease(std::get<NoteResult>(result));
			break;
		}
	}
}

void GameTrack::OnKeyDown() {
	if (m_callback) {
		m_callback(m_laneIndex, true);
	}

	std::vector<Note*> copy = m_notes;
	for (auto& it : copy) {
		auto result = it->CheckHit();

		if (std::get<bool>(result)) {
			it->OnHit(std::get<NoteResult>(result));
			break;
		}
	}
}

void GameTrack::AddNote(NoteInfoDesc* desc) {
	Note* note = nullptr;
	if (m_noteCaches.size() > 0) {
		note = m_noteCaches.back();
		m_noteCaches.pop_back();
	}
	else {
		note = new Note(m_engine);
	}

	note->Load(desc);
	note->SetXPosition(m_laneOffset);

	m_notes.push_back(note);
}

void GameTrack::ListenEvent(std::function<void(int, bool)> callback) {
	m_callback = callback;
}
