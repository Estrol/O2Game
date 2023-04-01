#include "Note.hpp"
#include "../../Engine/EstEngine.hpp"

#include "RhythmEngine.hpp"
#include "DrawableNote.hpp"
#include "NoteImageCacheManager.hpp"
#include "GameAudioSampleCache.hpp"

#define REMOVE_TIME 3000

namespace {
	float GetNotePositionAlpha(double Offset, double InitialTrackPosition, double HitPosOffset, double noteSpeed, bool upscroll = false) {
		double pos = (HitPosOffset + ((InitialTrackPosition - Offset) * (upscroll ? noteSpeed : -noteSpeed) / 100.0)) / 1000.0;
		return -(1 - 0.8) + pos;
	}
}

Note::Note(RhythmEngine* engine) {
	m_engine = engine;

	m_imageType = NoteImageType::WHITE;
	m_imageBodyType = NoteImageType::HOLD_WHITE;
	
	m_head = nullptr;
	m_tail = nullptr;
	m_body = nullptr;

	m_startTime = 0;
	m_endTime = 0;
	m_type = NoteType::NORMAL;
	m_lane = 0;
	m_initialTrackPosition = 0;
	m_endTrackPosition = 0;

	m_keysoundIndex = -1;
	m_state = NoteState::NORMAL_NOTE;

	m_drawAble = false;
	m_laneOffset = 0;
	m_removeAble = false;

	m_currentSample = nullptr;
	m_didHitHead = false;
	m_didHitTail = false;
}

Note::~Note() {
	Release();
}

void Note::Load(NoteInfoDesc* desc) {
	m_imageType = desc->ImageType;
	m_imageBodyType = desc->ImageBodyType;

	m_head = NoteImageCacheManager::GetInstance()->Depool(m_imageType);
	if (desc->Type == NoteType::HOLD) {
		m_tail = NoteImageCacheManager::GetInstance()->Depool(m_imageType);
		m_body = NoteImageCacheManager::GetInstance()->DepoolTile(m_imageBodyType);

		m_state = NoteState::HOLD_PRE;
	}
	else {
		m_tail = nullptr;
		m_body = nullptr;

		m_state = NoteState::NORMAL_NOTE;
	}

	m_startTime = desc->StartTime;
	m_endTime = desc->EndTime;
	m_type = desc->Type;
	m_lane = desc->Lane;
	m_initialTrackPosition = desc->InitialTrackPosition;
	m_endTrackPosition = desc->EndTrackPosition;
	m_keysoundIndex = desc->KeysoundIndex;

	m_laneOffset = 0;
	m_drawAble = false;
	m_removeAble = false;

	m_currentSample = nullptr;
	m_didHitHead = false;
	m_didHitTail = false;
}

void Note::Update(double delta) {
	if (IsRemoveable()) return;

	double audioPos = m_engine->GetGameAudioPosition();

	if (m_type == NoteType::NORMAL) {
		if (audioPos > (m_startTime + REMOVE_TIME)) {
			m_state = NoteState::DO_REMOVE;
		}
	}
	else {
		if (m_state == NoteState::HOLD_PRE) {
			if (audioPos > (m_startTime + REMOVE_TIME)) {
				m_state = NoteState::HOLD_MISSED_ACTIVE;
			}
		}
		else if (
			m_state == NoteState::HOLD_ON_HOLDING || 
			m_state == NoteState::HOLD_MISSED_ACTIVE || 
			m_state == NoteState::HOLD_PASSED) {
			if (audioPos > (m_endTime + REMOVE_TIME)) {
				m_state = NoteState::DO_REMOVE;
			}
		}
	}
}

void Note::Render(double delta) {
	if (IsRemoveable()) return;
	if (!m_drawAble) return;

	double trackPosition = m_engine->GetTrackPosition();
	float a1 = GetNotePositionAlpha(trackPosition, m_initialTrackPosition, 1000, m_engine->GetNotespeed());
	float a2 = 0.0f;

	if (m_endTrackPosition != -1) {
		a2 = GetNotePositionAlpha(trackPosition, m_endTrackPosition, 1000, m_engine->GetNotespeed());
	}

	UDim2 startOffset = UDim2::fromOffset(0, 0);
	UDim2 endOffset = UDim2::fromOffset(0, 600);

	if (m_type == NoteType::HOLD) {
		m_head->Position = UDim2::fromOffset(m_laneOffset, 0) + startOffset.Lerp(endOffset, a1);
		m_tail->Position = UDim2::fromOffset(m_laneOffset, 0) + startOffset.Lerp(endOffset, a2);

		if (m_state == NoteState::HOLD_ON_HOLDING) {
			m_head->Position.Y.Offset = 480;
		}

		m_head->CalculateSize();
		m_tail->CalculateSize();

		double headPos = m_head->AbsolutePosition.Y + (m_head->AbsoluteSize.Y / 2.0);
		double tailPos = m_tail->AbsolutePosition.Y + (m_tail->AbsoluteSize.Y / 2.0);

		double height = headPos - tailPos;
		double position = (height / 2.0) + tailPos;

		m_body->Position = UDim2::fromOffset(m_laneOffset, position);
		m_body->Size = { 1, 0, 0, height };

		if (!m_didHitTail) {
			m_body->Draw(false);
			m_head->Draw(false);
			m_tail->Draw(false);
		}
	}
	else {
		m_head->Position = UDim2::fromOffset(m_laneOffset, 0) + startOffset.Lerp(endOffset, a1);
		m_head->Draw(false);
	}
}

double Note::GetInitialTrackPosition() const {
	return m_initialTrackPosition;
}

std::tuple<bool, NoteResult> Note::CheckHit() {
	if (m_type == NoteType::NORMAL) {
		double time_to_end = m_engine->GetGameAudioPosition() - m_startTime;
		return TimeToResult(time_to_end);
	}
	else {
		if (m_state == NoteState::HOLD_PRE) {
			double time_to_end = m_engine->GetGameAudioPosition() - m_startTime;
			return TimeToResult(time_to_end);
		}
		else if (m_state == NoteState::HOLD_MISSED_ACTIVE) {
			double time_to_end = m_engine->GetGameAudioPosition() - m_endTime;
			return TimeToResult(time_to_end);
		}

		return { false, NoteResult::MISS };
	}
}

std::tuple<bool, NoteResult> Note::CheckRelease() {
	if (m_type == NoteType::HOLD) {
		double time_to_end = m_engine->GetGameAudioPosition() - m_endTime;

		if (m_state == NoteState::HOLD_ON_HOLDING || m_state == NoteState::HOLD_MISSED_ACTIVE) {
			auto result = TimeToResult(time_to_end);

			if (std::get<bool>(result)) {
				return result;
			}

			if (m_state == NoteState::HOLD_ON_HOLDING) {
				return { false, NoteResult::MISS };
			}
			else {
				return { true, NoteResult::MISS };
			}
		}
	}

	return { false, NoteResult::MISS };
}

void Note::OnHit(NoteResult result) {
	m_currentSample = GameAudioSampleCache::Play(m_keysoundIndex, 50);

	if (m_type == NoteType::HOLD) {
		if (m_state == NoteState::HOLD_PRE) {
			m_didHitHead = true;
			// m_currentResult
			m_state = NoteState::HOLD_ON_HOLDING;
		}
		else if (m_state == NoteState::HOLD_MISSED_ACTIVE) {
			m_didHitHead = true;
			m_state = NoteState::HOLD_PASSED;
		}
	}
	else {
		m_state = NoteState::DO_REMOVE;
		// TODO: scoring
	}
}

void Note::OnRelease(NoteResult result) {
	if (m_type == NoteType::HOLD) {
		if (m_state == NoteState::HOLD_ON_HOLDING || m_state == NoteState::HOLD_MISSED_ACTIVE) {
			if (result == NoteResult::MISS) {
				if (m_currentSample) {
					m_currentSample->Stop();
					m_currentSample = nullptr;
				}

				m_state = NoteState::HOLD_MISSED_ACTIVE;
			}
			else {
				m_state = NoteState::HOLD_PASSED;
				m_didHitTail = true;
			}
		}
	}
}

void Note::SetXPosition(int x) {
	m_laneOffset = x;
}

void Note::SetDrawable(bool drawable) {
	m_drawAble = drawable;
}

bool Note::IsDrawable() {
	if (m_removeAble) return false;

	return m_drawAble;
}

bool Note::IsRemoveable() {
	return m_state == NoteState::DO_REMOVE;
}

void Note::Release() {
	m_state = NoteState::DO_REMOVE;
	m_removeAble = true;

	if (m_type == NoteType::HOLD) {
		NoteImageCacheManager::GetInstance()->Repool(m_head, m_imageType);
		m_head = nullptr;

		NoteImageCacheManager::GetInstance()->Repool(m_tail, m_imageType);
		m_tail = nullptr;

		NoteImageCacheManager::GetInstance()->RepoolTile(m_body, m_imageBodyType);
		m_body = nullptr;
	}
	else {
		NoteImageCacheManager::GetInstance()->Repool(m_head, m_imageType);
		m_head = nullptr;
	}
}