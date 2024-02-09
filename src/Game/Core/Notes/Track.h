/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "Note.h"
#include <Inputs/Keys.h>
#include <functional>
#include <iostream>

struct NoteHitInfo;

struct TrackEvent
{
    int Lane = -1;

    bool State = false;
    bool IsKeyEvent = false;
    bool IsHitEvent = false;
    bool IsHitLongEvent = false;
};

class Track
{
public:
    Track(RhythmEngine *engine, int laneIndex, int offset);
    ~Track();

    void Update(double delta);
    void Render(double delta);
    void OnKeyUp(double time);
    void OnKeyDown(double time);

    void HandleScore(NoteHitInfo info);
    void HandleHoldScore(HoldResult res);

    void AddNote(NoteInfoDesc note);
    void ListenEvent(std::function<void(TrackEvent)> callback);

private:

    std::vector<std::shared_ptr<Note>> m_notes;
    std::vector<std::shared_ptr<Note>> m_noteCaches;
    std::vector<std::shared_ptr<Note>> m_inactive_notes;

    RhythmEngine *m_engine;
    int           m_laneOffset;
    int           m_laneIndex;

    int m_keySound;
    int m_keyVolume;
    int m_keyPan;

    double m_deleteDelay;

    std::shared_ptr<Note> m_currentHold;
    bool                  m_onHold;

    std::function<void(TrackEvent)> m_callback;
};