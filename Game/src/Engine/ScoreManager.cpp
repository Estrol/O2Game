#include "ScoreManager.hpp"
#include <algorithm>
#include <limits.h>
#include "../EnvironmentSetup.hpp"

ScoreManager::ScoreManager()
{
    m_cool = 0;
    m_good = 0;
    m_bad = 0;
    m_miss = 0;

    m_score = 0;

    m_jamCombo = 0;
    m_maxCombo = 0;

    m_combo = 0;
    m_coolCombo = 0;

    m_jamGauge = 0;
    m_maxJamCombo = 0;

    m_numOfPills = 0;
    m_life = 100.0f;

    m_lnCombo = 0;
    m_lnMaxCombo = 0;
}

ScoreManager::~ScoreManager()
{
}

void ScoreManager::OnHit(NoteHitInfo info) // Fuck it, just leave it max HP 100 and divided that by 10
{
    int difficulty = EnvironmentSetup::GetInt("Difficulty");
    switch (difficulty) {
    case 0: // Easy
        switch (info.Result) {
        case NoteResult::COOL:
            AddLife(0.3f);
            m_jamGauge += 4.0f;
            m_score += 200 + (10 * m_jamCombo);
            m_cool++;
            break;
        case NoteResult::GOOD:
            AddLife(0.2f);
            m_jamGauge += 2.0f;
            m_score += 100 + (5 * m_jamCombo);
            m_good++;
            break;
        case NoteResult::BAD: // IDK if this correct or not (from old code)
            if (m_numOfPills > 0) {
                m_numOfPills = std::clamp(m_numOfPills - 1, 0, 5);
                m_score += 200 + (10 * m_jamCombo);
                m_cool++;
                info.Result = NoteResult::COOL;
            }
            else {
                AddLife(-1.0f);
                m_jamGauge = 0.0f;
                m_coolCombo = 0;
                m_score += 4;
                m_combo = 0;
                m_bad++;
            }
            break;
        default:
            AddLife(-5.0f);
            m_combo = 0;
            m_jamCombo = 0;
            m_jamGauge = 0.0f;
            if (m_life > 0) {
                m_score -= 10;
            }
            m_miss++;
            break;
        }
        break;

    case 1: // Normal
        switch (info.Result) {
        case NoteResult::COOL:
            AddLife(0.2f);
            m_jamGauge += 4.0f;
            m_score += 200 + (10 * m_jamCombo);
            m_cool++;
            break;
        case NoteResult::GOOD:
            AddLife(0.1f);
            m_jamGauge += 2.0f;
            m_score += 100 + (5 * m_jamCombo);
            m_good++;
            break;
        case NoteResult::BAD: // IDK if this correct or not (from old code)
            if (m_numOfPills > 0) {
                m_numOfPills = std::clamp(m_numOfPills - 1, 0, 5);
                m_score += 200 + (10 * m_jamCombo);
                m_cool++;
                info.Result = NoteResult::COOL;
            }
            else {
                AddLife(-0.7f);
                m_jamGauge = 0.0f;
                m_coolCombo = 0;
                m_score += 4;
                m_combo = 0;
                m_bad++;
            }
            break;
        default:
            AddLife(-4.0f);
            m_combo = 0;
            m_jamCombo = 0;
            m_jamGauge = 0.0f;
            if (m_life > 0) {
                m_score -= 10;
            }
            m_miss++;
            break;
        }
        break;

    case 2: // Hard
        switch (info.Result) {
        case NoteResult::COOL:
            AddLife(0.1f);
            m_jamGauge += 4.0f;
            m_score += 200 + (10 * m_jamCombo);
            m_cool++;
            break;
        case NoteResult::GOOD:
            AddLife(0.0f);
            m_jamGauge += 2.0f;
            m_score += 100 + (5 * m_jamCombo);
            m_good++;
            break;
        case NoteResult::BAD: // IDK if this correct or not (from old code)
            if (m_numOfPills > 0) {
                m_numOfPills = std::clamp(m_numOfPills - 1, 0, 5);
                m_score += 200 + (10 * m_jamCombo);
                m_cool++;
                info.Result = NoteResult::COOL;
            }
            else {
                AddLife(-0.5f);
                m_jamGauge = 0.0f;
                m_coolCombo = 0;
                m_score += 4;
                m_combo = 0;
                m_bad++;
            }
            break;
        default:
            AddLife(-3.0f);
            m_combo = 0;
            m_jamCombo = 0;
            m_jamGauge = 0.0f;
            if (m_life > 0) {
                m_score -= 10;
            }
            m_miss++;
            break;
        }
        break;
    }

    m_combo = std::clamp(m_combo, 0, INT_MAX);
    m_jamCombo = std::clamp(m_jamCombo, 0, INT_MAX);
    m_jamGauge = std::clamp(m_jamGauge, 0.0f, 100.0f);
    m_score = std::clamp(m_score, 0, INT_MAX);

    if (info.Result == NoteResult::COOL) {
        m_coolCombo += 1;

        if (m_coolCombo > 15) {
            m_coolCombo = 0;
            m_numOfPills = std::clamp(m_numOfPills + 1, 0, 5);
        }
    }
    else {
        m_coolCombo = 0;
    }

    if (info.Result != NoteResult::BAD && info.Result != NoteResult::MISS) {
        m_combo += 1;
        m_maxCombo = std::max(m_maxCombo, m_combo);
    }

    if (m_jamGauge >= kMaxJamGauge) {
        m_jamGauge = 0;
        m_jamCombo += 1;
        m_maxJamCombo = std::max(m_maxJamCombo, m_jamCombo);

        if (m_jamCallback) {
            m_jamCallback(m_jamCombo);
        }
    }

    if (m_callback) {
        if (info.Result == NoteResult::MISS && m_life == 0) {
            return;
        }

        m_callback(info);
    }
}

void ScoreManager::OnLongNoteHold(HoldResult result)
{
    switch (result) {
    case HoldResult::HoldBreak:
    {
        m_lnCombo = 0;
        break;
    }

    case HoldResult::HoldAdd:
    {
        m_lnCombo += 1;
        break;
    }
    }

    m_lnMaxCombo = (std::max)(m_lnCombo, m_lnMaxCombo);

    if (m_lncallback) {
        m_lncallback();
    }
}

void ScoreManager::ListenHit(std::function<void(NoteHitInfo)> cb)
{
    m_callback = cb;
}

void ScoreManager::ListenJam(std::function<void(int)> cb)
{
    m_jamCallback = cb;
}

void ScoreManager::ListenLongNote(std::function<void()> cb)
{
    m_lncallback = cb;
}

int ScoreManager::GetPills() const
{
    return m_numOfPills;
}

float ScoreManager::GetLife() const
{
    return m_life;
}

float ScoreManager::GetJamGauge() const
{
    return m_jamGauge;
}

std::tuple<int, int, int, int, int, int, int, int, int, int, int> ScoreManager::GetScore() const
{
    return { m_score, m_cool, m_good, m_bad, m_miss, m_jamCombo, m_maxJamCombo, m_combo, m_maxCombo, m_lnCombo, m_lnMaxCombo };
}

void ScoreManager::AddLife(float sz)
{
    if (m_life > 0) {
        m_life += sz;
        m_life = std::clamp(m_life, 0.0f, 100.0f);
    }
}