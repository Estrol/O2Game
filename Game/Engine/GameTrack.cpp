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
	m_keySound = -1;
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

			/*if (note->GetType() == NoteType::HOLD && !note->IsHoldEffectDrawable()) {
				if (m_callback) {
					GameTrackEvent e = {};
					e.Lane = m_laneIndex;
					e.State = false;
					e.IsHitLongEvent = true;
					e.IsHitEvent = true;

					m_callback(e);
				}
			}*/

			if (note->GetStartTime() <= m_engine->GetGameAudioPosition()) {
				m_keySound = note->GetKeysoundId();
			}

			note->Update(delta);
			_note++;
		}
	}
}

void GameTrack::Render(double delta) {
	for (auto& note : m_notes) {
		note->Render(delta);
	}
}

void GameTrack::OnKeyUp() {
	if (m_callback) {
		GameTrackEvent e = {};
		e.Lane = m_laneIndex;
		e.State = false;
		e.IsKeyEvent = true;

		m_callback(e);
	}

	for (auto& note : m_notes) {
		auto result = note->CheckRelease();
		if (std::get<bool>(result)) {
			note->OnRelease(std::get<NoteResult>(result));

			if (std::get<NoteResult>(result) == NoteResult::MISS) {
				GameAudioSampleCache::Stop(note->GetKeysoundId());
			}
			
			m_currentHold = nullptr;
			break;
		}
	}
}

void GameTrack::OnKeyDown() {
	if (m_callback) {
		GameTrackEvent e = {};
		e.Lane = m_laneIndex;
		e.State = true;
		e.IsKeyEvent = true;

		m_callback(e);
	}

	bool found = false;
	for (auto& note : m_notes) {
		auto result = note->CheckHit();
		if (std::get<bool>(result)) {
			note->OnHit(std::get<NoteResult>(result));

			if (note->GetType() == NoteType::HOLD) {
				m_currentHold = note;
			}

			GameAudioSampleCache::Play(note->GetKeysoundId(), 50);
			found = true;
			break;
		}
	}

	if (!found) {
		GameAudioSampleCache::Play(m_keySound, 50);
	}
}

void GameTrack::HandleScore(NoteHitInfo info) {
	if (!info.Ignore) {
		if (info.IsRelease) {
			if (m_callback) {
				GameTrackEvent e = {};
				e.Lane = m_laneIndex;
				e.State = false;
				e.IsHitLongEvent = true;
				e.IsHitEvent = true;

				m_callback(e);
			}
		}
		else {
			if (m_callback) {
				GameTrackEvent e = {};
				e.Lane = m_laneIndex;
				e.State = true;
				e.IsHitLongEvent = info.Type == 2;
				e.IsHitEvent = true;

				m_callback(e);
			}
		}
	}

	m_engine->GetScoreManager()->OnHit(info);
}

void GameTrack::HandleHoldScore(HoldResult res) {
	m_engine->GetScoreManager()->OnLongNoteHold(res);
}

void GameTrack::AddNote(NoteInfoDesc* desc) {
	Note* note = nullptr;
	if (m_noteCaches.size() > 0) {
		note = m_noteCaches.back();
		m_noteCaches.pop_back();
	}
	else {
		note = new Note(m_engine, this);
	}

	note->Load(desc);
	note->SetXPosition(m_laneOffset);

	if (m_keySound == -1) {
		m_keySound = note->GetKeysoundId();
	}

	m_notes.push_back(note);
}

void GameTrack::ListenEvent(std::function<void(GameTrackEvent)> callback) {
	m_callback = callback;
}
