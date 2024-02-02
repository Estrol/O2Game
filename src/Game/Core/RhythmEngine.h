/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include "./Judgements/JudgeBase.h"
#include "./Notes/TimingLineManager.h"
#include "./Notes/Track.h"
#include "./Replay/Autoplay.h"
#include "./Scoring/ScoreManager.h"
#include "./Timings/TimingBase.h"
#include <Graphics/Utils/Rect.h>
#include <Inputs/Keys.h>
#include <Math/Vector2.h>
#include <vector>

enum class GameState {
    PreParing,
    NotGame,
    PreGame,
    Playing,
    PosGame
};

struct ManiaKeyState
{
    Inputs::Keys key;
    bool         isPressed;
};

struct ReplayFrameData
{
    std::vector<Autoplay::ReplayHitInfo> KeyDowns;
    std::vector<Autoplay::ReplayHitInfo> KeyUps;
};

struct RhythmGameInfo
{
    Chart                               *Chart;
    std::vector<Autoplay::ReplayHitInfo> ReplayData;
    std::vector<Inputs::Keys>            Keybinds;
};

class RhythmEngine
{
public:
    RhythmEngine();
    ~RhythmEngine();

    bool Load(RhythmGameInfo info);

    bool Start();
    bool Stop();
    bool Ready();

    void Update(double delta);
    void Draw(double delta);
    void DrawTimingLine(double delta);

    void OnKeyDown(const Inputs::State &state);
    void OnKeyUp(const Inputs::State &state);

    void ListenKeyEvent(std::function<void(TrackEvent)> callback);
    void SetKeys(Inputs::Keys *keys);

    double GetAudioPosition() const;
    double GetVisualPosition() const;
    double GetGameAudioPosition() const;
    double GetTrackPosition() const;
    double GetPrebufferTiming() const;
    double GetNotespeed() const;
    double GetBPMAt(double offset) const;
    double GetAudioLength() const;

    const float *GetLaneSizes() const;
    const float *GetLanePos() const;

    void        SetHitPosition(int offset);
    void        SetLaneOffset(int offset);
    int         GetHitPosition() const;
    Vector2     GetResolution() const;
    Rect        GetPlayRectangle() const;
    std::string GetTitle() const;

    GameState               GetState() const;
    TimingBase             *GetTiming() const;
    JudgeBase              *GetJudge() const;
    ScoreManager           *GetScoreManager() const;
    std::vector<TimingInfo> GetBPMs() const;
    std::vector<TimingInfo> GetSVs() const;

    double GetElapsedTime() const;
    int    GetPlayTime() const;
    int    GetNoteImageIndex();

    int  GetGuideLineIndex() const;
    void SetGuideLineIndex(int idx);

private:
    void            UpdateNotes();
    void            UpdateGamePosition();
    void            UpdateVirtualResolution();
    void            CreateTimingMarkers();
    ReplayFrameData GetAutoplayAtThisFrame(double offset);

    double m_rate = 0.0;
    double m_offset = 0.0;
    double m_beatmapOffset = 0.0;
    double m_currentAudioPosition = 0.0;
    double m_currentVisualPosition = 0.0;
    double m_currentAudioGamePosition = 0.0;
    double m_currentTrackPosition = 0.0;
    float  m_baseBPM, m_currentBPM = 0.0;
    float  m_currentSVMultiplier = 0.0;

    int    m_currentSampleIndex = 0;
    int    m_currentNoteIndex = 0;
    int    m_currentBPMIndex = 0;
    int    m_currentSVIndex = 0;
    int    m_scrollSpeed = 0;
    double m_audioLength = 0;
    int    m_hitPosition = 0;
    int    m_laneOffset = 0;
    int    m_audioVolume = 100;
    int    m_audioOffset = 0;

    int m_noteImageIndex = 0;
    int m_noteMaxImageIndex = 0;

    int m_guideLineIndex = 0;
    int m_autoMinIndex = 0;

    bool m_started = false;
    bool m_is_autoplay = false;

    GameState   m_state = GameState::NotGame;
    std::string m_title;

    Rect          m_playRectangle;
    float         m_laneSize[7];
    float         m_lanePos[7];
    ManiaKeyState m_keybinds[7];

    std::filesystem::path m_audioPath = "";
    Chart                *m_currentChart;
    Vector2               m_virtualResolution = { 0, 0 };
    Vector2               m_gameResolution = { 0, 0 };

    std::vector<double>                                           m_timingPositionMarkers;
    std::vector<Track *>                                          m_tracks;
    std::vector<NoteInfoDesc>                                     m_noteDescs;
    std::vector<AutoSample>                                       m_autoSamples;
    std::unordered_map<int, int>                                  m_autoHitIndex;
    std::unordered_map<int, std::vector<Autoplay::ReplayHitInfo>> m_autoHitInfos;
    std::vector<Autoplay::ReplayHitInfo>                          m_autoFrames;

    int                                   m_PlayTime = 0;
    std::chrono::system_clock::time_point m_startClock;

    std::shared_ptr<TimingBase>        m_timings = nullptr;
    std::shared_ptr<JudgeBase>         m_judge = nullptr;
    std::shared_ptr<ScoreManager>      m_scoreManager = nullptr;
    std::shared_ptr<TimingLineManager> m_timingLineManager = nullptr;
    std::function<void(TrackEvent)>    m_eventCallback = nullptr;
};