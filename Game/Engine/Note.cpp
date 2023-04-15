#include "Note.hpp"
#include "../../Engine/EstEngine.hpp"

#include "RhythmEngine.hpp"
#include "DrawableNote.hpp"
#include "NoteImageCacheManager.hpp"
#include "GameAudioSampleCache.hpp"

#define REMOVE_TIME 800
#define HOLD_COMBO_TICK 100

namespace {
	float GetNotePositionAlpha(double Offset, double InitialTrackPosition, double HitPosOffset, double noteSpeed, bool upscroll = false) {
		double pos = (HitPosOffset + ((InitialTrackPosition - Offset) * (upscroll ? noteSpeed : -noteSpeed) / 100.0)) / 1000.0;
		return -(1 - 0.8) + pos;
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

	if (m_type == NoteType::NORMAL) {
		if (audioPos > (m_startTime + kNoteBadHitWindowMax)) {
			m_state = NoteState::DO_REMOVE;

			m_hitPos = m_startTime + kNoteBadHitWindowMax;
			//m_engine->GetScoreManager()->OnHit({ NoteResult::MISS, m_startTime + kNoteBadHitWindowMax });
			OnHit(NoteResult::MISS);
		}
	}
	else {
		if (m_state == NoteState::HOLD_PRE) {
			if (audioPos > (m_startTime + kNoteBadHitWindowMax)) {
				m_state = NoteState::HOLD_MISSED_ACTIVE;

				m_hitPos = m_startTime + kNoteBadHitWindowMax;
				//m_engine->GetScoreManager()->OnHit({ NoteResult::MISS, m_hitPos });
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
						//m_engine->GetScoreManager()->OnLongNoteHold(HoldResult::HoldAdd);
					}
				}
			}

			if (audioPos > (m_endTime + kNoteBadHitWindowMax)) {
				if (m_state == NoteState::HOLD_ON_HOLDING || m_state == NoteState::HOLD_MISSED_ACTIVE) {
					m_hitPos = m_endTime + kNoteBadHitWindowMax;
					//m_didHitTail = true;

					//m_engine->GetScoreManager()->OnHit({ NoteResult::MISS, m_relPos });
					//m_engine->GetScoreManager()->OnLongNoteHold(HoldResult::HoldBreak);
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

double Note::GetStartTime() const {
	return m_startTime;
}

int Note::GetKeysoundId() const {
	return m_keysoundIndex;
}

NoteType Note::GetType() const {
	return m_type;
}

std::tuple<bool, NoteResult> Note::CheckHit() {
	if (m_type == NoteType::NORMAL) {
		double time_to_end = m_engine->GetGameAudioPosition() - m_startTime;
		return TimeToResult(m_engine, m_startTime, time_to_end);
	}
	else {
		if (m_state == NoteState::HOLD_PRE) {
			double time_to_end = m_engine->GetGameAudioPosition() - m_startTime;
			return TimeToResult(m_engine, m_startTime, time_to_end);
		}
		else if (m_state == NoteState::HOLD_MISSED_ACTIVE) {
			double time_to_end = m_engine->GetGameAudioPosition() - m_endTime;
			return TimeToResult(m_engine, m_endTime, time_to_end);
		}

		return { false, NoteResult::MISS };
	}
}

std::tuple<bool, NoteResult> Note::CheckRelease() {
	if (m_type == NoteType::HOLD) {
		double time_to_end = m_engine->GetGameAudioPosition() - m_endTime;

		if (m_state == NoteState::HOLD_ON_HOLDING || m_state == NoteState::HOLD_MISSED_ACTIVE) {
			auto result = TimeToResult(m_engine, m_endTime, time_to_end);

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
			//m_engine->GetScoreManager()->OnLongNoteHold(HoldResult::HoldAdd);
			m_lastScoreTime = m_engine->GetGameAudioPosition();
			//m_engine->GetScoreManager()->OnHit({ result, m_hitPos });

			m_track->HandleHoldScore(HoldResult::HoldAdd);
			m_track->HandleScore({
				result,
				m_hitPos,
				false,
				2
			});
		}
		else if (m_state == NoteState::HOLD_MISSED_ACTIVE) {
			m_didHitHead = true;
			m_state = NoteState::HOLD_PASSED;

			//m_engine->GetScoreManager()->OnLongNoteHold(HoldResult::HoldBreak);
			//m_engine->GetScoreManager()->OnHit({ result, m_hitPos });

			m_track->HandleHoldScore(HoldResult::HoldBreak);
			m_track->HandleScore({
				result,
				m_hitPos,
				true,
				2
			});
		}
	}
	else {
		m_state = NoteState::DO_REMOVE;
		//m_engine->GetScoreManager()->OnHit({ result, m_hitPos });
		m_track->HandleScore({
				result,
				m_hitPos,
				false,
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
				//m_engine->GetScoreManager()->OnLongNoteHold(HoldResult::HoldBreak);
				//m_engine->GetScoreManager()->OnHit({ result, m_relPos });
				m_state = NoteState::HOLD_MISSED_ACTIVE;

				m_track->HandleHoldScore(HoldResult::HoldBreak);
				m_track->HandleScore({
					result,
					m_hitPos,
					true,
					2
					});
			}
			else {
 				m_state = NoteState::HOLD_PASSED;
				m_didHitTail = true;
				//m_engine->GetScoreManager()->OnHit({ result, m_relPos });
				m_track->HandleScore({
					result,
					m_hitPos,
					true,
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