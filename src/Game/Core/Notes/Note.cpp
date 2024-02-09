/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../Audio/SampleManager.h"
#include "../RhythmEngine.h"
#include "Note.h"
#include "NoteCacheManager.h"
#include "Track.h"

#define REMOVE_TIME 800
#define HOLD_COMBO_TICK 100

namespace {
    double CalculateNotePosition(double offset, double initialTrackPos, double hitPosition, double noteSpeed, bool upscroll)
    {
        return hitPosition + ((initialTrackPos - offset) * (upscroll ? noteSpeed : -noteSpeed) / 100);
    }

    double lerp(double min, double max, float alpha)
    {
        return min * (1.0 - alpha) + (max * alpha);
    }

    bool isWithinRange(double point, double minRange, double maxRange)
    {
        return (point >= minRange && point <= maxRange);
    }

    bool isCollision(double top, double bottom, double min, double max)
    {
        if (top >= min && top <= max) {
            return true;
        }

        if (bottom >= min && bottom <= max) {
            return true;
        }

        if (top <= min && bottom >= max) {
            return true;
        }

        return false;
    }

    const int length_multiplier[4] = {
        0,
        2,
        4,
        6
    };
} // namespace

Note::Note(RhythmEngine *engine, Track *track)
{
    m_Engine = engine;
    m_Track = track;

    m_ImageType = NoteImageType::LANE_1;
    m_ImageBodyType = NoteImageType::HOLD_LANE_1;

    m_Head = nullptr;
    m_Tail = nullptr;
    m_Body = nullptr;

    m_StartTime = 0;
    m_EndTime = 0;
    m_StartBPM = 0;
    m_EndBPM = 0;

    m_Type = NoteType::NORMAL;
    m_Lane = 0;
    m_InitialTrackPosition = 0;
    m_EndTrackPosition = 0;

    m_KeysoundIndex = -1;
    m_State = NoteState::NORMAL_NOTE;

    m_Drawable = false;
    m_LaneOffset = 0;
    m_Removeable = false;

    m_DidHitHead = false;
    m_DidHitTail = false;
    m_ShouldDrawHoldEffect = true;

    m_HitPos = 0;
    m_RelPos = 0;

    m_KeyVolume = 50;
    m_KeyPan = 0;

    m_LastScoreTime = -1;
}

Note::~Note()
{
}

void Note::Load(NoteInfoDesc desc)
{
    m_ImageType = desc.ImageType;
    m_ImageBodyType = desc.ImageBodyType;

    m_Head = NoteCacheManager::Get()->Depool(m_ImageType);
    m_Head->AnchorPoint = { 0, 1 };

    if (desc.Type == NoteType::HOLD) {
        m_Tail = NoteCacheManager::Get()->Depool(m_ImageType);
        m_Tail->AnchorPoint = m_Head->AnchorPoint;

        m_Body = NoteCacheManager::Get()->DepoolHold(m_ImageBodyType);
        m_Body->AnchorPoint = { 0, 0.5 };

        m_StartBPM = desc.StartBPM;
        m_EndBPM = desc.EndBPM;
        m_State = NoteState::HOLD_PRE;
    } else {
        m_Tail = nullptr;
        m_Body = nullptr;

        m_StartBPM = desc.StartBPM;
        m_EndBPM = 0;
        m_State = NoteState::NORMAL_NOTE;
    }

    m_TrailUp = NoteCacheManager::Get()->DepoolTrail(NoteImageType::TRAIL_UP);
    m_TrailDown = NoteCacheManager::Get()->DepoolTrail(NoteImageType::TRAIL_DOWN);

    m_StartTime = desc.StartTime;
    m_EndTime = desc.EndTime;
    m_Type = desc.Type;
    m_Lane = desc.Lane;
    m_InitialTrackPosition = desc.InitialTrackPosition;
    m_EndTrackPosition = desc.EndTrackPosition;
    m_KeysoundIndex = desc.KeysoundIndex;

    m_KeyVolume = desc.Volume;
    m_KeyPan = desc.Pan;

    m_LaneOffset = 0;
    m_Drawable = false;
    m_Removeable = false;
    m_Ignore = true;

    m_DidHitHead = false;
    m_DidHitTail = false;
    m_ShouldDrawHoldEffect = true;

    m_HitPos = 0;
    m_RelPos = 0;

    m_LastScoreTime = -1;
}

void Note::Update(double delta)
{
    if (IsRemoveable()) {
        return;
    }

    JudgeBase *judge = m_Engine->GetJudge();
    double     audioPos = m_Engine->GetGameAudioPosition();
    m_HitTime = m_StartTime - audioPos;

    if (m_Type == NoteType::NORMAL) {
        if (judge->IsMissed(this, audioPos)) {
            if (!IsPassed()) {
                m_HitPos = m_StartTime + m_HitTime;
                OnHit(NoteResult::MISS);
            }

            m_State = NoteState::DO_REMOVE;
        }
    } else {
        if (m_State == NoteState::HOLD_PRE) {
            if (judge->IsMissed(this, audioPos)) {
                m_HitPos = m_StartTime + m_HitTime;
                OnHit(NoteResult::MISS);
            }
        } else if (
            m_State == NoteState::HOLD_ON_HOLDING ||
            m_State == NoteState::HOLD_MISSED_ACTIVE ||
            m_State == NoteState::HOLD_PASSED) {

            if (m_State == NoteState::HOLD_ON_HOLDING) {
                if (m_LastScoreTime != -1 && audioPos <= m_EndTime && audioPos > m_StartTime) {
                    if (audioPos - m_LastScoreTime > HOLD_COMBO_TICK) {
                        m_LastScoreTime += HOLD_COMBO_TICK;
                        m_Track->HandleHoldScore(HoldResult::HoldAdd);
                    }
                }
            }

            if (judge->IsMissed(this, audioPos)) {
                if (m_State == NoteState::HOLD_ON_HOLDING || m_State == NoteState::HOLD_MISSED_ACTIVE) {
                    m_HitPos = m_EndTime + (m_EndTime - audioPos);
                    if (m_State == NoteState::HOLD_ON_HOLDING) {
                        OnRelease(NoteResult::MISS);
                    }
                }

                m_State = NoteState::DO_REMOVE;
            }
        }
    }
}

void Note::Render(double delta)
{
    if (IsRemoveable()) {
        return;
    }

    if (!m_Drawable) {
        return;
    }

    auto   resolution = m_Engine->GetResolution();
    auto   hitPos = m_Engine->GetHitPosition();
    double trackPosition = m_Engine->GetTrackPosition();

    int  min = -100, max = hitPos + 25;
    auto playRect = m_Engine->GetPlayRectangle();

    int guideLineIndex = m_Engine->GetGuideLineIndex();

    int guideLineLength = 24 * length_multiplier[guideLineIndex];

    if (m_Type == NoteType::HOLD) {
        double y1 = CalculateNotePosition(trackPosition, m_InitialTrackPosition, 1000.0, m_Engine->GetNotespeed(), false) / 1000.0;
        double y2 = CalculateNotePosition(trackPosition, m_EndTrackPosition, 1000.0, m_Engine->GetNotespeed(), false) / 1000.0;

        m_Head->Position = UDim2::fromOffset(m_LaneOffset, lerp(0.0, (double)hitPos, (float)y1));
        m_Tail->Position = UDim2::fromOffset(m_LaneOffset, lerp(0.0, (double)hitPos, (float)y2));

        float Transparency = 0.9f;

        if (m_HitResult >= NoteResult::GOOD && m_State == NoteState::HOLD_ON_HOLDING) {
            // m_Head->Position.Y.Offset = hitPos;
            Transparency = 1.0f;
        }

        m_Head->CalculateSize();
        m_Tail->CalculateSize();

        double headPos = m_Head->AbsolutePosition.Y + (m_Head->AbsoluteSize.Y / 2.0);
        double tailPos = m_Tail->AbsolutePosition.Y + (m_Tail->AbsoluteSize.Y / 2.0);

        double height = headPos - tailPos;
        double position = (height / 2.0) + tailPos;

        m_Body->Position = UDim2::fromOffset(m_LaneOffset, position);
        m_Body->Size = { 0, m_Body->Size.X.Offset, 0, height };

        m_Body->TintColor = { Transparency, Transparency, Transparency };

        bool b1 = isWithinRange(m_Head->Position.Y.Offset, min, max);
        bool b2 = isWithinRange(m_Tail->Position.Y.Offset, min, max);

        if (isCollision(m_Tail->Position.Y.Offset, m_Head->Position.Y.Offset, min, max)) {
            m_Body->SetIndexAt(m_Engine->GetNoteImageIndex());
            m_Body->Draw(delta, playRect);
        }

        if (b1) {
            if (guideLineLength > 0) {
                m_TrailDown->Position = m_Head->Position;
                m_TrailDown->Size = UDim2::fromOffset(1, guideLineLength);
                m_TrailDown->AnchorPoint = { 0, 0 };
                m_TrailDown->Draw(delta, playRect);

                m_TrailDown->Position = m_Head->Position + UDim2::fromOffset(m_Head->AbsoluteSize.X, 0);
                m_TrailDown->AnchorPoint = { 1, 0 };
                m_TrailDown->Draw(delta, playRect);
            }

            m_Head->SetIndexAt(m_Engine->GetNoteImageIndex());
            m_Head->Draw(delta, playRect);
        }

        if (b2) {
            if (guideLineLength > 0) {
                m_TrailUp->Position = m_Tail->Position + UDim2::fromOffset(0, -m_Tail->AbsoluteSize.Y);
                m_TrailUp->Size = UDim2::fromOffset(1, guideLineLength);
                m_TrailUp->AnchorPoint = { 0, 1 };
                m_TrailUp->Draw(delta, playRect);

                m_TrailUp->Position = m_Tail->Position + UDim2::fromOffset(m_Tail->AbsoluteSize.X, -m_Tail->AbsoluteSize.Y);
                m_TrailUp->AnchorPoint = { 1, 1 };
                m_TrailUp->Draw(delta, playRect);
            }

            m_Tail->SetIndexAt(m_Engine->GetNoteImageIndex());
            m_Tail->Draw(delta, playRect);
        }
    } else {
        double y1 = CalculateNotePosition(trackPosition, m_InitialTrackPosition, 1000.0, m_Engine->GetNotespeed(), false) / 1000.0;
        m_Head->Position = UDim2::fromOffset(m_LaneOffset, lerp(0.0, (double)hitPos, (float)y1));
        m_Head->CalculateSize();

        bool b1 = isWithinRange(m_Head->Position.Y.Offset, min, max);

        if (b1) {
            if (guideLineLength > 0) {
                m_TrailDown->Position = m_Head->Position;
                m_TrailDown->Size = UDim2::fromOffset(1, guideLineLength);
                m_TrailDown->AnchorPoint = { 0, 0 };
                m_TrailDown->Draw(delta, playRect);

                m_TrailDown->Position = m_Head->Position + UDim2::fromOffset(m_Head->AbsoluteSize.X, 0);
                m_TrailDown->AnchorPoint = { 1, 0 };
                m_TrailDown->Draw(delta, playRect);

                m_TrailUp->Position = m_Head->Position + UDim2::fromOffset(0, -m_Head->AbsoluteSize.Y);
                m_TrailUp->Size = UDim2::fromOffset(1, guideLineLength);
                m_TrailUp->AnchorPoint = { 0, 1 };
                m_TrailUp->Draw(delta, playRect);

                m_TrailUp->Position = m_Head->Position + UDim2::fromOffset(m_Head->AbsoluteSize.X, -m_Head->AbsoluteSize.Y);
                m_TrailUp->AnchorPoint = { 1, 1 };
                m_TrailUp->Draw(delta, playRect);
            }

            m_Head->SetIndexAt(m_Engine->GetNoteImageIndex());
            m_Head->Draw(delta, playRect);
        }
    }
}

double Note::GetInitialTrackPosition() const
{
    return m_InitialTrackPosition;
}

double Note::GetStartTime() const
{
    return m_StartTime;
}

double Note::GetBPMTime() const
{
    if (GetType() == NoteType::HOLD) {
        if (m_State == NoteState::HOLD_PRE) {
            return m_StartBPM;
        } else {
            return m_EndBPM;
        }
    } else {
        return m_StartBPM;
    }
}

double Note::GetHitTime() const
{
    if (GetType() == NoteType::HOLD) {
        if (m_State == NoteState::HOLD_PRE) {
            return m_StartTime;
        } else {
            return m_EndTime;
        }
    } else {
        return m_StartTime;
    }
}

int Note::GetKeysoundId() const
{
    return m_KeysoundIndex;
}

int Note::GetKeyVolume() const
{
    return m_KeyVolume;
}

int Note::GetKeyPan() const
{
    return m_KeyPan;
}

NoteType Note::GetType() const
{
    return m_Type;
}

void Note::SetXPosition(int x)
{
    m_LaneOffset = x;
}

void Note::SetDrawable(bool drawable)
{
    m_Drawable = drawable;
}

bool Note::IsHoldEffectDrawable() const
{
    return m_ShouldDrawHoldEffect;
}

std::tuple<bool, NoteResult> Note::CheckHit(double time)
{
    JudgeBase *judge = m_Engine->GetJudge();

    if (m_Type == NoteType::NORMAL) {
        double time_to_end = time - m_StartTime;
        auto   result = judge->CalculateResult(this, time);
        if (std::get<bool>(result)) {
            m_Ignore = false;
        }

        return result;
    } else {
        if (m_State == NoteState::HOLD_PRE) {
            double time_to_end = time - m_StartTime;
            auto   result = judge->CalculateResult(this, time);
            if (std::get<bool>(result)) {
                m_Ignore = false;
            }

            return result;
        } else if (m_State == NoteState::HOLD_MISSED_ACTIVE) {
            double time_to_end = time - m_EndTime;
            auto   result = judge->CalculateResult(this, time);
            if (std::get<bool>(result)) {
                m_Ignore = false;
            }

            return result;
        }

        return { false, NoteResult::MISS };
    }
}

std::tuple<bool, NoteResult> Note::CheckRelease(double time)
{
    if (m_Type == NoteType::HOLD) {
        double     time_to_end = time - m_EndTime;
        JudgeBase *judge = m_Engine->GetJudge();

        if (m_State == NoteState::HOLD_ON_HOLDING || m_State == NoteState::HOLD_MISSED_ACTIVE) {
            auto result = judge->CalculateResult(this, time);

            if (std::get<bool>(result)) {
                if (m_State == NoteState::HOLD_MISSED_ACTIVE) {
                    return { true, NoteResult::BAD };
                }

                return result;
            }

            if (m_State == NoteState::HOLD_ON_HOLDING) {
                return { true, NoteResult::MISS };
            } else {
                return { false, NoteResult::MISS };
            }
        }
    }

    return { false, NoteResult::MISS };
}

void Note::OnHit(NoteResult result)
{
    if (m_Type == NoteType::HOLD) {
        if (m_State == NoteState::HOLD_PRE) {
            m_DidHitHead = true;

            m_LastScoreTime = m_Engine->GetGameAudioPosition();

            if (result == NoteResult::MISS) {
                m_State = NoteState::HOLD_MISSED_ACTIVE;

                m_Track->HandleHoldScore(HoldResult::HoldBreak);
            } else {
                m_State = NoteState::HOLD_ON_HOLDING;

                m_Track->HandleHoldScore(HoldResult::HoldAdd);
            }

            m_HitResult = result;
            m_Track->HandleScore({ result,
                                   m_HitPos,
                                   false,
                                   m_Ignore,
                                   2 });
        } else if (m_State == NoteState::HOLD_MISSED_ACTIVE) {
            m_DidHitHead = true;
            m_State = NoteState::HOLD_PASSED;

            m_Track->HandleHoldScore(HoldResult::HoldBreak);
            m_Track->HandleScore({ result,
                                   m_HitPos,
                                   true,
                                   m_Ignore,
                                   2 });
        }
    } else {
        m_State = NoteState::NORMAL_NOTE_PASSED;
        m_Track->HandleScore({ result,
                               m_HitPos,
                               false,
                               m_Ignore,
                               1 });
    }
}

void Note::OnRelease(NoteResult result)
{
    if (m_Type == NoteType::HOLD) {
        if (m_State == NoteState::HOLD_ON_HOLDING || m_State == NoteState::HOLD_MISSED_ACTIVE) {
            m_LastScoreTime = -1;

            if (result == NoteResult::MISS) {
                SampleManager::Stop(m_KeysoundIndex);
                m_State = NoteState::HOLD_MISSED_ACTIVE;

                m_Track->HandleHoldScore(HoldResult::HoldBreak);
                m_Track->HandleScore({ result,
                                       m_HitPos,
                                       true,
                                       m_Ignore,
                                       2 });
            } else {
                m_State = NoteState::HOLD_PASSED;
                m_DidHitTail = true;
                m_Track->HandleScore({ result,
                                       m_HitPos,
                                       true,
                                       m_Ignore,
                                       2 });
            }
        }
    }
}

bool Note::IsDrawable() const
{
    if (m_Removeable)
        return false;

    return m_Drawable;
}

bool Note::IsRemoveable() const
{
    return m_State == NoteState::DO_REMOVE;
}

bool Note::IsPassed() const
{
    return m_State == NoteState::NORMAL_NOTE_PASSED || m_State == NoteState::HOLD_PASSED;
}

bool Note::IsHeadHit() const
{
    return m_DidHitHead;
}

bool Note::IsTailHit() const
{
    return m_DidHitTail;
}

NoteState Note::GetState() const
{
    return m_State;
}

void Note::Release()
{
    m_State = NoteState::DO_REMOVE;
    m_Removeable = true;

    auto cacheManager = NoteCacheManager::Get();

    cacheManager->RepoolTrail(m_TrailDown, NoteImageType::TRAIL_DOWN);
    cacheManager->RepoolTrail(m_TrailUp, NoteImageType::TRAIL_UP);

    m_TrailDown = nullptr;
    m_TrailUp = nullptr;

    if (m_Type == NoteType::HOLD) {
        cacheManager->Repool(m_Head, m_ImageType);
        m_Head = nullptr;

        cacheManager->Repool(m_Tail, m_ImageType);
        m_Tail = nullptr;

        cacheManager->RepoolHold(m_Body, m_ImageBodyType);
        m_Body = nullptr;
    } else {
        cacheManager->Repool(m_Head, m_ImageType);
        m_Head = nullptr;
    }
}