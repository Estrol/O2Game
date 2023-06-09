#include "Note.hpp"
#include "../../Engine/EstEngine.hpp"

#include "RhythmEngine.hpp"
#include "DrawableNote.hpp"
#include "NoteImageCacheManager.hpp"
#include "GameAudioSampleCache.hpp"

#define REMOVE_TIME 800
#define HOLD_COMBO_TICK 100

namespace {
	double CalculateNotePosition(double offset, double initialTrackPos, double hitPosition, double noteSpeed, bool upscroll) {
		return hitPosition + ((initialTrackPos - offset) * (upscroll ? noteSpeed : -noteSpeed) / 100);
	}

	bool isWithinRange(int point, int minRange, int maxRange) {
		return (point >= minRange && point <= maxRange);
	}

	bool isCollision(int top, int bottom, int min, int max) {
		// Check if the top value of the rectangle is within the range
		if (top >= min && top <= max) {
			return true;  // Collision detected
		}

		// Check if the bottom value of the rectangle is within the range
		if (bottom >= min && bottom <= max) {
			return true;  // Collision detected
		}

		// Check if the range is completely inside the rectangle
		if (top <= min && bottom >= max) {
			return true;  // Collision detected
		}

		// No collision detected
		return false;
	}
}

Note::Note(RhythmEngine* engine, GameTrack* track) {
	m_engine = engine;
	m_track = track;

	m_imageType = NoteImageType::WHITE;
	m_imageBodyType = NoteImageType::HOLD_WHITE;
	
	m_head = nullptr;
	m_tail = nullptr;
	m_body = nullptr;

	m_startTime = 0;
	m_endTime = 0;
	m_startBPM = 0;
	m_endBPM = 0;

	m_type = NoteType::NORMAL;
	m_lane = 0;
	m_initialTrackPosition = 0;
	m_endTrackPosition = 0;

	m_keysoundIndex = -1;
	m_state = NoteState::NORMAL_NOTE;

	m_drawAble = false;
	m_laneOffset = 0;
	m_removeAble = false;

	m_didHitHead = false;
	m_didHitTail = false;
	m_shouldDrawHoldEffect = true;

	m_hitPos = 0;
	m_relPos = 0;

	m_keyVolume = 50;
	m_keyPan = 0;

	m_lastScoreTime = -1;
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

		m_startBPM = desc->StartBPM;
		m_endBPM = desc->EndBPM;
		m_state = NoteState::HOLD_PRE;
	}
	else {
		m_tail = nullptr;
		m_body = nullptr;

		m_startBPM = desc->StartBPM;
		m_endBPM = 0;
		m_state = NoteState::NORMAL_NOTE;
	}

	m_startTime = desc->StartTime;
	m_endTime = desc->EndTime;
	m_type = desc->Type;
	m_lane = desc->Lane;
	m_initialTrackPosition = desc->InitialTrackPosition;
	m_endTrackPosition = desc->EndTrackPosition;
	m_keysoundIndex = desc->KeysoundIndex;

	m_keyVolume = desc->Volume;
	m_keyPan = desc->Pan;

	m_laneOffset = 0;
	m_drawAble = false;
	m_removeAble = false;
	m_ignore = true;

	m_didHitHead = false;
	m_didHitTail = false;
	m_shouldDrawHoldEffect = true;

	m_hitPos = 0;
	m_relPos = 0;

	m_lastScoreTime = -1;
}

void Note::Update(double delta) {
	if (IsRemoveable()) return;

	double audioPos = m_engine->GetGameAudioPosition();
	m_hitTime = m_startTime - audioPos; //static_cast<double>(SDL_GetPerformanceFrequency()) / 1000000.0;

	if (m_type == NoteType::NORMAL) {
		//if (audioPos > (m_startTime + kNoteBadHitWindowMax)) {
		if (IsMissed(m_engine, this)) {
			m_state = NoteState::DO_REMOVE;

			m_hitPos = m_startTime + kNoteBadHitWindowMax;
			OnHit(NoteResult::MISS);
		}
	}
	else {
		if (m_state == NoteState::HOLD_PRE) {
			//if (audioPos > (m_startTime + kNoteBadHitWindowMax)) {
			if (IsMissed(m_engine, this)) {
				m_state = NoteState::HOLD_MISSED_ACTIVE;

				m_hitPos = m_startTime + kNoteBadHitWindowMax;
				OnHit(NoteResult::MISS);
			}
		}
		else if (
			m_state == NoteState::HOLD_ON_HOLDING || 
			m_state == NoteState::HOLD_MISSED_ACTIVE || 
			m_state == NoteState::HOLD_PASSED) {
			if (m_state == NoteState::HOLD_ON_HOLDING) {
				if (m_lastScoreTime != -1 && audioPos <= m_endTime && audioPos > m_startTime) {
					if (audioPos - m_lastScoreTime > HOLD_COMBO_TICK) {
						m_lastScoreTime += HOLD_COMBO_TICK;
						m_track->HandleHoldScore(HoldResult::HoldAdd);
					}
				}
			}

			//if (audioPos > (m_endTime + kNoteBadHitWindowMax)) {
			if (IsMissed(m_engine, this)) {
				if (m_state == NoteState::HOLD_ON_HOLDING || m_state == NoteState::HOLD_MISSED_ACTIVE) {
					m_hitPos = m_endTime + kNoteBadHitWindowMax;
					if (m_state == NoteState::HOLD_ON_HOLDING) {
						OnRelease(NoteResult::MISS);
					}
				}

				m_state = NoteState::DO_REMOVE;
			}
		}
	}
}

void Note::Render(double delta) {
	if (IsRemoveable()) return;
	if (!m_drawAble) return;

	auto resolution = m_engine->GetResolution();
	auto hitPos = m_engine->GetHitPosition();
	double trackPosition = m_engine->GetTrackPosition();

	int min = 0, max = hitPos + 10;
	auto playRect = m_engine->GetPlayRectangle();

	if (m_type == NoteType::HOLD) {
		double y1 = CalculateNotePosition(trackPosition, m_initialTrackPosition, hitPos, m_engine->GetNotespeed(), false);
		double y2 = CalculateNotePosition(trackPosition, m_endTrackPosition, hitPos, m_engine->GetNotespeed(), false);

		m_head->Position = UDim2::fromOffset(m_laneOffset, y1);
		m_tail->Position = UDim2::fromOffset(m_laneOffset, y2) ;

		float Transparency = 0.9f;

		if (m_hitResult >= NoteResult::GOOD && m_state == NoteState::HOLD_ON_HOLDING) {
			m_head->Position.Y.Offset = hitPos;
			Transparency = 1.0f;
		}

		m_head->CalculateSize();
		m_tail->CalculateSize();

		double headPos = m_head->AbsolutePosition.Y + (m_head->AbsoluteSize.Y / 2.0);
		double tailPos = m_tail->AbsolutePosition.Y + (m_tail->AbsoluteSize.Y / 2.0);

		double height = headPos - tailPos;
		double position = (height / 2.0) + tailPos;

		m_body->Position = UDim2::fromOffset(m_laneOffset, position);
		m_body->Size = { 1, 0, 0, height };
		
		m_body->TintColor = { Transparency, Transparency, Transparency };

		bool b1 = isWithinRange(m_head->Position.Y.Offset, min, max);
		bool b2 = isWithinRange(m_tail->Position.Y.Offset, min, max);

		if (isCollision(m_tail->Position.Y.Offset, m_head->Position.Y.Offset, min, max)) {
			m_body->Draw(&playRect);
		}

		if (b1) {
			m_head->Draw(&playRect);
		}

		if (b2) {
			m_tail->Draw(&playRect);
		}
	}
	else {
		double y1 = CalculateNotePosition(trackPosition, m_initialTrackPosition, hitPos, m_engine->GetNotespeed(), false);
		m_head->Position = UDim2::fromOffset(m_laneOffset, y1);
		
		bool b1 = isWithinRange(m_head->Position.Y.Offset, min, max);

		if (b1) {
			m_head->Draw(&playRect);
		}
	}
}

double Note::GetInitialTrackPosition() const {
	return m_initialTrackPosition;
}

double Note::GetStartTime() const {
	return m_startTime;
}

double Note::GetBPMTime() const {
	if (GetType() == NoteType::HOLD) {
		if (m_state == NoteState::HOLD_PRE) {
			return m_startBPM;
		}
		else {
			return m_endBPM;
		}
	}
	else {
		return m_startBPM;
	}
}

double Note::GetHitTime() const {
	if (GetType() == NoteType::HOLD) {
		if (m_state == NoteState::HOLD_PRE) {
			return m_startTime;
		}
		else {
			return m_endTime;
		}
	}
	else {
		return m_startTime;
	}
}

int Note::GetKeysoundId() const {
	return m_keysoundIndex;
}

int Note::GetKeyVolume() const {
	return m_keyVolume;	
}

int Note::GetKeyPan() const {
	return m_keyPan;
}

NoteType Note::GetType() const {
	return m_type;
}

std::tuple<bool, NoteResult> Note::CheckHit() {
	if (m_type == NoteType::NORMAL) {
		double time_to_end = m_engine->GetGameAudioPosition() - m_startTime;
		auto result = TimeToResult(m_engine, this);
		if (std::get<bool>(result)) {
			m_ignore = false;
		}

		return result;
	}
	else {
		if (m_state == NoteState::HOLD_PRE) {
			double time_to_end = m_engine->GetGameAudioPosition() - m_startTime;
			auto result = TimeToResult(m_engine, this);
			if (std::get<bool>(result)) {
				m_ignore = false;
			}

			return result;
		}
		else if (m_state == NoteState::HOLD_MISSED_ACTIVE) {
			double time_to_end = m_engine->GetGameAudioPosition() - m_endTime;
			auto result = TimeToResult(m_engine, this);
			if (std::get<bool>(result)) {
				m_ignore = false;
			}

			return result;
		}

		return { false, NoteResult::MISS };
	}
}

std::tuple<bool, NoteResult> Note::CheckRelease() {
	if (m_type == NoteType::HOLD) {
		double time_to_end = m_engine->GetGameAudioPosition() - m_endTime;

		if (m_state == NoteState::HOLD_ON_HOLDING || m_state == NoteState::HOLD_MISSED_ACTIVE) {
			auto result = TimeToResult(m_engine, this);

			if (std::get<bool>(result)) {
				if (m_state == NoteState::HOLD_MISSED_ACTIVE) {
					return { true, NoteResult::BAD };
				}

				return result;
			}

			if (m_state == NoteState::HOLD_ON_HOLDING) {
				return { true, NoteResult::MISS };
			}
			else {
				return { false, NoteResult::MISS };
			}
		}
	}

	return { false, NoteResult::MISS };
}

void Note::OnHit(NoteResult result) {
	if (m_type == NoteType::HOLD) {
		if (m_state == NoteState::HOLD_PRE) {
			m_didHitHead = true;
			m_state = NoteState::HOLD_ON_HOLDING;
			m_lastScoreTime = m_engine->GetGameAudioPosition();

			m_hitResult = result;
			m_track->HandleHoldScore(HoldResult::HoldAdd);
			m_track->HandleScore({
				result,
				m_hitPos,
				false,
				m_ignore,
				2
			});
		}
		else if (m_state == NoteState::HOLD_MISSED_ACTIVE) {
			m_didHitHead = true;
			m_state = NoteState::HOLD_PASSED;

			m_track->HandleHoldScore(HoldResult::HoldBreak);
			m_track->HandleScore({
				result,
				m_hitPos,
				true,
				m_ignore,
				2
			});
		}
	}
	else {
		m_state = NoteState::DO_REMOVE;
		m_track->HandleScore({
			result,
			m_hitPos,
			false,
			m_ignore,
			1
		});
	}
}

void Note::OnRelease(NoteResult result) {
	if (m_type == NoteType::HOLD) {
		if (m_state == NoteState::HOLD_ON_HOLDING || m_state == NoteState::HOLD_MISSED_ACTIVE) {
			m_lastScoreTime = -1;

			if (result == NoteResult::MISS) {
				GameAudioSampleCache::Stop(m_keysoundIndex);
				m_state = NoteState::HOLD_MISSED_ACTIVE;

				m_track->HandleHoldScore(HoldResult::HoldBreak);
				m_track->HandleScore({
					result,
					m_hitPos,
					true,
					m_ignore,
					2
				});
			}
			else {
 				m_state = NoteState::HOLD_PASSED;
				m_didHitTail = true;
				m_track->HandleScore({
					result,
					m_hitPos,
					true,
					m_ignore,
					2
				});
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

bool Note::IsHoldEffectDrawable() {
	return m_shouldDrawHoldEffect;
}

bool Note::IsDrawable() {
	if (m_removeAble) return false;

	return m_drawAble;
}

bool Note::IsRemoveable() {
	return m_state == NoteState::DO_REMOVE;
}

bool Note::IsHeadHit() {
	return m_didHitHead;
}

bool Note::IsTailHit() {
	return m_didHitTail;
}

void Note::Release() {
	m_state = NoteState::DO_REMOVE;
	m_removeAble = true;

	auto cacheManager = NoteImageCacheManager::GetInstance();

	if (m_type == NoteType::HOLD) {
		cacheManager->Repool(m_head, m_imageType);
		m_head = nullptr;

		cacheManager->Repool(m_tail, m_imageType);
		m_tail = nullptr;

		cacheManager->RepoolTile(m_body, m_imageBodyType);
		m_body = nullptr;
	}
	else {
		cacheManager->Repool(m_head, m_imageType);
		m_head = nullptr;
	}
}