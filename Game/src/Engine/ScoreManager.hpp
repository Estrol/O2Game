#pragma once
#include "Judgements/NoteResult.h"
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

    int                                                               GetPills() const;
    float                                                             GetLife() const;
    float                                                             GetJamGauge() const;
    std::tuple<int, int, int, int, int, int, int, int, int, int, int> GetScore() const;

private:
    void AddLife(float sz);

    int m_score;
    int m_cool;
    int m_good;
    int m_bad;
    int m_miss;
    int m_jamCombo;
    int m_maxJamCombo;
    int m_combo;
    int m_maxCombo;

    int m_numOfPills;
    int m_coolCombo;

    float m_jamGauge;
    float m_life;

    int m_lnCombo;
    int m_lnMaxCombo;

    std::function<void(NoteHitInfo)> m_callback;
    std::function<void()>            m_lncallback;
    std::function<void(int)>         m_jamCallback;
};