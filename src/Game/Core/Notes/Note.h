/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

#include "../Enums/NoteImageType.h"
#include "../Judgements/JudgeBase.h"
#include <tuple>

class Sprite;
class RhythmEngine;
class Track;
enum class NoteType : uint8_t;

enum class NoteState {
    NORMAL_NOTE,
    NORMAL_NOTE_PASSED,

    HOLD_PRE,
    HOLD_MISSED_ACTIVE,
    HOLD_ON_HOLDING,
    HOLD_PASSED,

    DO_REMOVE
};

struct NoteInfoDesc
{
    NoteImageType ImageType;
    NoteImageType ImageBodyType;

    int KeysoundIndex;
    int Volume;
    int Pan;

    double StartTime;
    double EndTime;
    double StartBPM;
    double EndBPM;

    int      Lane;
    NoteType Type;

    double InitialTrackPosition;
    double EndTrackPosition;
};

class Note
{
public:
    Note(RhythmEngine *engine, Track *track);
    ~Note();

    void Load(NoteInfoDesc desc);

    void Update(double delta);
    void Render(double delta);

    double GetInitialTrackPosition() const;
    double GetStartTime() const;
    double GetBPMTime() const;
    double GetHitTime() const;

    int       GetKeysoundId() const;
    int       GetKeyVolume() const;
    int       GetKeyPan() const;
    NoteType  GetType() const;
    NoteState GetState() const;

    std::tuple<bool, NoteResult> CheckHit();
    std::tuple<bool, NoteResult> CheckRelease();
    void                         OnHit(NoteResult result);
    void                         OnRelease(NoteResult result);

    void SetXPosition(int x);
    void SetDrawable(bool drawable);

    bool IsHoldEffectDrawable() const;
    bool IsRemoveable() const;
    bool IsDrawable() const;
    bool IsPassed() const;

    bool IsHeadHit() const;
    bool IsTailHit() const;

    void Release();

private:
    bool m_Drawable;
    bool m_Removeable;

    RhythmEngine *m_Engine;
    Track        *m_Track;

    Sprite *m_Head;
    Sprite *m_Body;
    Sprite *m_Tail;

    Sprite *m_TrailUp;
    Sprite *m_TrailDown;

    NoteImageType m_ImageType;
    NoteImageType m_ImageBodyType;

    int    m_LaneOffset;
    double m_StartTime;
    double m_EndTime;
    double m_StartBPM;
    double m_EndBPM;

    int m_KeysoundIndex;
    int m_Lane;

    int m_KeyVolume;
    int m_KeyPan;

    NoteType   m_Type;
    NoteState  m_State;
    NoteResult m_HitResult;

    double m_InitialTrackPosition;
    double m_EndTrackPosition;

    bool m_ShouldDrawHoldEffect;
    bool m_DidHitHead;
    bool m_DidHitTail;
    bool m_Ignore = true;

    double m_HitTime;
    double m_HitPos;
    double m_RelPos;

    double m_LastScoreTime;
};