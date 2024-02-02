/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "ScoreManager.h"
#include <Logs.h>
#include <algorithm>
#include <limits.h>

ScoreManager::ScoreManager()
{
    memset(&m_Score, 0, sizeof(Score));

    m_Score.Life = 100.0f;
}

ScoreManager::~ScoreManager()
{
}

void ScoreManager::OnHit(NoteHitInfo info)
{
    switch (info.Result) {

        case NoteResult::COOL:
        {
            AddLife(0.1f);
            m_Score.JamGauge += 4;
            m_Score.Score += 200 + (10 * m_Score.JamCombo);
            m_Score.Cool++;

            break;
        }

        case NoteResult::GOOD:
        {
            AddLife(0.0f);
            m_Score.JamGauge += 2;
            m_Score.Score += 100 + (5 * m_Score.JamCombo);
            m_Score.Good++;

            break;
        }

        case NoteResult::BAD:
        {
            if (m_Score.NumOfPills > 0) {
                m_Score.NumOfPills = std::clamp(m_Score.NumOfPills - 1, 0, 5);
                m_Score.Score += 200 + (10 * m_Score.JamCombo);
                m_Score.Cool++;

                info.Result = NoteResult::COOL;
            } else {
                AddLife(-0.5f);
                m_Score.JamGauge = 0;
                m_Score.CoolCombo = 0;
                m_Score.Score += 4;
                m_Score.Combo = 0;
                m_Score.Bad++;
            }

            break;
        }

        case NoteResult::MISS:
        {
            AddLife(-3.0f);
            m_Score.Score = 0;
            m_Score.JamCombo = 0;
            m_Score.JamGauge = 0;

            if (m_Score.Life > 0) {
                m_Score.Score -= 10;
            }

            m_Score.Combo = 0;
            m_Score.Miss++;
            break;
        }

        default:
        {
            Logs::Puts("Unknown NoteResult: %d", info.Result);
            break;
        }
    }

    m_Score.Combo = std::clamp(m_Score.Combo, 0, INT_MAX);
    m_Score.JamCombo = std::clamp(m_Score.JamCombo, 0, INT_MAX);
    m_Score.JamGauge = std::clamp(m_Score.JamGauge, 0.0f, 100.0f);
    m_Score.Score = std::clamp(m_Score.Score, 0, INT_MAX);

    if (info.Result == NoteResult::COOL) {
        m_Score.CoolCombo += 1;

        if (m_Score.CoolCombo > 15) {
            m_Score.CoolCombo = 0;
            m_Score.NumOfPills = std::clamp(m_Score.NumOfPills + 1, 0, 5);
        }
    } else {
        m_Score.CoolCombo = 0;
    }

    if (info.Result != NoteResult::BAD && info.Result != NoteResult::MISS) {
        m_Score.Combo += 1;
        m_Score.MaxCombo = std::max(m_Score.MaxCombo, m_Score.Combo);
    }

    if (m_Score.JamGauge >= kMaxJamGauge) {
        m_Score.JamGauge = 0;
        m_Score.JamCombo += 1;
        m_Score.MaxJamCombo = std::max(m_Score.MaxJamCombo, m_Score.JamCombo);

        if (m_jamCallback) {
            m_jamCallback(m_Score.JamCombo);
        }
    }

    if (m_callback) {
        if (info.Result == NoteResult::MISS && m_Score.Life <= 0) {
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
            m_Score.LnCombo = 0;
            break;
        }

        case HoldResult::HoldAdd:
        {
            m_Score.LnCombo += 1;
            break;
        }
    }

    m_Score.LnMaxCombo = (std::max)(m_Score.LnCombo, m_Score.LnMaxCombo);

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
    return m_Score.NumOfPills;
}

float ScoreManager::GetLife() const
{
    return m_Score.Life;
}

float ScoreManager::GetJamGauge() const
{
    return m_Score.JamGauge;
}

const Score &ScoreManager::GetScore() const
{
    return m_Score;
}

void ScoreManager::AddLife(float sz)
{
    if (m_Score.Life > 0) {
        m_Score.Life += sz;
        m_Score.Life = std::clamp(m_Score.Life, 0.0f, 100.0f);
    }
}
