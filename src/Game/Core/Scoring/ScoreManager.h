/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "../Judgements/NoteResult.h"
#include <functional>

constexpr float kMaxJamGauge = 100;
constexpr float kMaxLife = 100;

struct NoteHitInfo
{
    NoteResult Result;
    double     TimeToEnd;
    bool       IsRelease;
    bool       Ignore;
    int        Type;
};

struct Score
{
    int Score;
    int Cool;
    int Good;
    int Bad;
    int Miss;
    int JamCombo;
    int MaxJamCombo;
    int Combo;
    int MaxCombo;

    int NumOfPills;
    int CoolCombo;

    float JamGauge;
    float Life;

    int LnCombo;
    int LnMaxCombo;
};

class ScoreManager
{
public:
    ScoreManager();
    ~ScoreManager();

    void OnHit(NoteHitInfo info);
    void OnLongNoteHold(HoldResult result);
    void ListenHit(std::function<void(NoteHitInfo)>);
    void ListenJam(std::function<void(int)>);
    void ListenLongNote(std::function<void()>);

    int          GetPills() const;
    float        GetLife() const;
    float        GetJamGauge() const;
    const Score &GetScore() const;

private:
    void AddLife(float sz);

    Score m_Score;

    std::function<void(NoteHitInfo)> m_callback;
    std::function<void()>            m_lncallback;
    std::function<void(int)>         m_jamCallback;
};