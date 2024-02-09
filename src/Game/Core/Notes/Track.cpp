/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../Audio/SampleManager.h"
#include "../RhythmEngine.h"
#include "Track.h"
#include <algorithm>
#include <mutex>

// Max Object per lane
constexpr int kMaxObjectCount = 500;

Track::Track(RhythmEngine *engine, int laneIndex, int offset)
{
    m_engine = engine;
    m_laneIndex = laneIndex;
    m_laneOffset = offset;
    m_deleteDelay = 0;
    m_keySound = -1;
}

Track::~Track()
{
    for (auto &note : m_notes) {
        note.reset();
    }

    for (auto &note : m_noteCaches) {
        note.reset();
    }

    for (auto &note : m_inactive_notes) {
        note.reset();
    }

    m_inactive_notes.clear();
    m_currentHold.reset();
    m_noteCaches.clear();
    m_notes.clear();
}

void Track::Update(double delta)
{
    int ObjectCount = 0;
    int InactiveObjectCount = 0;

    double trackPos = m_engine->GetTrackPosition();
    double gameAudioPos = m_engine->GetGameAudioPosition();
    double preBufferPos = m_engine->GetPrebufferTiming();

    for (auto &note : m_notes) {
        if (++ObjectCount < kMaxObjectCount) {
            if (!note->IsPassed()) {
                if (!note->IsDrawable()) {
                    double startTime = note->GetInitialTrackPosition();

                    if (trackPos - startTime > preBufferPos) {
                        note->SetDrawable(true);
                    }
                }

                if (note->GetStartTime() <= gameAudioPos) {
                    m_keySound = note->GetKeysoundId();
                    m_keyVolume = note->GetKeyVolume();
                    m_keyPan = note->GetKeyPan();
                }

                note->Update(delta);
            } else {
                m_inactive_notes.push_back(note);
            }
        } else {
            break;
        }
    }

    for (auto &note : m_inactive_notes) {
        if (++InactiveObjectCount < kMaxObjectCount) {
            if (!note->IsRemoveable()) {
                note->Update(delta);
            } else {
                m_noteCaches.push_back(note);
            }
        } else {
            break;
        }
    }

    m_notes.erase(
        std::remove_if(m_notes.begin(), m_notes.end(), [](std::shared_ptr<Note> note) {
            return note->IsPassed();
        }),
        m_notes.end());

    m_inactive_notes.erase(
        std::remove_if(m_inactive_notes.begin(), m_inactive_notes.end(), [](std::shared_ptr<Note> note) {
            return note->IsRemoveable();
        }),
        m_inactive_notes.end());
}

void Track::Render(double delta)
{
    int ObjectCount = 0;

    for (auto &note : m_notes) {
        if (++ObjectCount < kMaxObjectCount) {
            note->Render(delta);
        } else {
            break;
        }
    }

    for (auto &note : m_inactive_notes) {
        if (++ObjectCount < kMaxObjectCount) {
            note->Render(delta);
        } else {
            break;
        }
    }
}

void Track::OnKeyUp(double time)
{
    if (m_callback) {
        TrackEvent e = {};
        e.Lane = m_laneIndex;
        e.State = false;
        e.IsKeyEvent = true;

        m_callback(e);
    }

    // create a copy of it, so it wont be NULL if the note is moved to cache
    for (std::shared_ptr<Note> note : m_notes) {
        if (note && !(note->IsPassed() && note->IsRemoveable())) {
            auto result = note->CheckRelease(time);
            if (std::get<bool>(result)) {
                note->OnRelease(std::get<NoteResult>(result));

                if (std::get<NoteResult>(result) == NoteResult::MISS) {
                    SampleManager::Stop(note->GetKeysoundId());
                }

                m_currentHold = nullptr;
                break;
            }
        }
    }
}

void Track::OnKeyDown(double time)
{
    if (m_callback) {
        TrackEvent e = {};
        e.Lane = m_laneIndex;
        e.State = true;
        e.IsKeyEvent = true;

        m_callback(e);
    }

    bool found = false;
    // create a copy of it, so it wont be NULL if the note is moved to cache
    for (std::shared_ptr<Note> note : m_notes) {
        if (note && !(note->IsPassed() && note->IsRemoveable())) {
            auto result = note->CheckHit(time);
            if (std::get<bool>(result)) {
                note->OnHit(std::get<NoteResult>(result));

                if (note->GetType() == NoteType::HOLD) {
                    m_currentHold = note;
                }

                SampleManager::Play(note->GetKeysoundId(), note->GetKeyVolume(), note->GetKeyPan());
                found = true;
                break;
            }
        }
    }

    if (!found) {
        SampleManager::Play(m_keySound, m_keyVolume, m_keyPan);
    }
}

void Track::HandleScore(NoteHitInfo info)
{
    if (!info.Ignore) {
        if (info.IsRelease) {
            if (m_callback) {
                TrackEvent e = {};
                e.Lane = m_laneIndex;
                e.State = false;
                e.IsHitLongEvent = true;
                e.IsHitEvent = true;

                m_callback(e);
            }
        } else {
            if (m_callback) {
                TrackEvent e = {};
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

void Track::HandleHoldScore(HoldResult res)
{
    m_engine->GetScoreManager()->OnLongNoteHold(res);
}

void Track::AddNote(NoteInfoDesc desc)
{
    std::shared_ptr<Note> note = nullptr;
    if (m_noteCaches.size() > 0) {
        note = m_noteCaches.back();
        m_noteCaches.pop_back();
    } else {
        note = std::make_shared<Note>(m_engine, this);
    }

    note->Load(desc);
    note->SetXPosition(m_laneOffset);

    if (m_keySound == -1) {
        m_keySound = note->GetKeysoundId();
    }

    m_notes.push_back(std::move(note));
}

void Track::ListenEvent(std::function<void(TrackEvent)> callback)
{
    m_callback = callback;
}
