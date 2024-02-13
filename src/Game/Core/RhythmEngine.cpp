/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "../Data/Util/Util.hpp"
#include "../Env.h"
#include "./Audio/SampleManager.h"
#include "./Resources/NoteImages.h"
#include "RhythmEngine.h"
#include <Configuration.h>
#include <Graphics/NativeWindow.h>
#include <Logs.h>

#include "./Timings/StaticTiming.h"
#include "./Timings/VelocityTiming.h"

#include "./Judgements/BeatBasedJudge.h"
#include "./Judgements/MsBasedJudge.h"

#define MAX_BUFFER_TXT_SIZE 256

namespace {
    std::vector<NoteImageType> Key2Type = {
        NoteImageType::LANE_1,
        NoteImageType::LANE_2,
        NoteImageType::LANE_3,
        NoteImageType::LANE_4,
        NoteImageType::LANE_5,
        NoteImageType::LANE_6,
        NoteImageType::LANE_7,
    };

    std::vector<NoteImageType> Key2HoldType = {
        NoteImageType::HOLD_LANE_1,
        NoteImageType::HOLD_LANE_2,
        NoteImageType::HOLD_LANE_3,
        NoteImageType::HOLD_LANE_4,
        NoteImageType::HOLD_LANE_5,
        NoteImageType::HOLD_LANE_6,
        NoteImageType::HOLD_LANE_7,
    };

    std::unordered_map<int, ManiaKeyState> KeyMapping = {
        { 0, { Inputs::Keys::A, false } },
        { 1, { Inputs::Keys::S, false } },
        { 2, { Inputs::Keys::D, false } },
        { 3, { Inputs::Keys::Space, false } },
        { 4, { Inputs::Keys::J, false } },
        { 5, { Inputs::Keys::K, false } },
        { 6, { Inputs::Keys::L, false } },
    };

    int trackOffset[] = { 5, 33, 55, 82, 114, 142, 164 };
} // namespace

RhythmEngine::RhythmEngine()
{
    m_currentAudioGamePosition = 0;
    m_currentVisualPosition = 0;
    m_currentTrackPosition = 0;
    m_rate = 1;
    m_offset = 0;
    m_scrollSpeed = 180;

    m_timingPositionMarkers = std::vector<double>();
}

RhythmEngine::~RhythmEngine()
{
    for (int i = 0; i < m_tracks.size(); i++) {
        delete m_tracks[i];
    }

    m_tracks.clear();
    m_timingPositionMarkers.clear();

    m_timings.reset();
    m_judge.reset();
    m_scoreManager.reset();
    m_timingLineManager.reset();

    Resources::NoteImages::UnloadImageResources();
}

bool RhythmEngine::Load(RhythmGameInfo info)
{
    Resources::NoteImages::LoadImageResources();

    {
        m_state = GameState::PreParing;
        m_currentChart = info.Chart;

        auto chart = info.Chart;

        m_autoHitIndex.clear();
        m_autoHitInfos.clear();

        // default is 99
        m_noteMaxImageIndex = 99;

        int currentX = m_laneOffset;
        for (int i = 0; i < 7; i++) {
            m_tracks.push_back(new Track(this, i, currentX));
            m_autoHitIndex[i] = 0;

            if (m_eventCallback) {
                m_tracks[i]->ListenEvent([&](TrackEvent e) {
                    m_eventCallback(e);
                });
            }

            auto noteTex = Resources::NoteImages::Get(Key2Type[i]);

            int size = noteTex->ImagesRect.Width;
            m_noteMaxImageIndex = (std::min)(noteTex->MaxFrames, m_noteMaxImageIndex);

            m_lanePos[i] = static_cast<float>(currentX);
            m_laneSize[i] = static_cast<float>(size);
            currentX += size;
        }

        currentX = static_cast<int>(accumulate(m_laneSize, m_laneSize + 7, 0));
        m_playRectangle = { m_laneOffset, 0, m_laneOffset + currentX, m_hitPosition };

        std::filesystem::path audioPath = chart->m_beatmapDirectory;
        audioPath /= chart->m_audio;

        if (Env::GetInt("Mirror")) {
            chart->ApplyMod(Mod::MIRROR);
        } else if (Env::GetInt("Random")) {
            chart->ApplyMod(Mod::RANDOM);
        } else if (Env::GetInt("Rearrange")) {
            void *lane_data = Env::GetPointer("LaneData");

            chart->ApplyMod(Mod::REARRANGE, lane_data);
        }

        if (Env::GetInt("NoSV")) {
            m_timings = std::make_shared<StaticTiming>(chart->m_bpms, chart->m_svs, chart->InitialSvMultiplier);
        } else {
            m_timings = std::make_shared<VelocityTiming>(chart->m_bpms, chart->m_svs, chart->InitialSvMultiplier);
        }

        if (Env::GetInt("StaticJudge")) {
            m_judge = std::make_shared<MsBasedJudge>(this);
        } else {
            m_judge = std::make_shared<BeatBasedJudge>(this);
        }

        if (std::filesystem::exists(audioPath) && audioPath.has_extension()) {
            m_audioPath = audioPath;
        }

        for (auto &sample : chart->m_autoSamples) {
            m_autoSamples.push_back(sample);
        }

        try {
            m_audioVolume = Configuration::GetInt("Game", "AudioVolume");
        } catch (const std::invalid_argument &) {
            Logs::Puts("[Gameplay] Invalid volume provided!");
            m_audioVolume = 100;
        }

        try {
            m_audioOffset = Configuration::GetInt("Game", "AudioOffset");
        } catch (const std::invalid_argument &) {
            Logs::Puts("[Gameplay] Invalid offset!");
            m_audioOffset = 0;
        }

        bool IsAutoSound = false;
        try {
            IsAutoSound = Configuration::GetBool("Game", "AutoSound");
        } catch (const std::invalid_argument &) {
            Logs::Puts("[Gameplay] Invalid auto sound bool value");
            IsAutoSound = false;
        }

        try {
            m_scrollSpeed = Configuration::GetInt("Game", "NoteSpeed");
        } catch (const std::invalid_argument &) {
            Logs::Puts("[Gameplay] Invalid notespeed, reverting to 210 value");
            m_scrollSpeed = 320;
        }

        m_rate = Env::GetFloat("SongRate");
        m_rate = std::clamp(m_rate, 0.05, 2.0);

        {
            m_title = chart->m_title;
            char buffer[MAX_BUFFER_TXT_SIZE];
            sprintf(buffer, "Lv.%d %s", chart->m_level, (const char *)chart->m_title.c_str());

            m_title = std::string(buffer);
            if (m_rate != 1.0) {
                memset(buffer, 0, MAX_BUFFER_TXT_SIZE);
                sprintf(buffer, "[%.2fx] %s", m_rate, (const char *)m_title.c_str());

                m_title = std::string(buffer);
            }
        }

        m_beatmapOffset = chart->m_bpms[0].StartTime;
        m_audioLength = chart->GetLength();
        m_baseBPM = chart->BaseBPM;
        m_currentBPM = m_baseBPM;
        m_currentSVMultiplier = chart->InitialSvMultiplier;

        CreateTimingMarkers();
        UpdateVirtualResolution();

        for (auto &note : chart->m_notes) {
            NoteInfoDesc desc = {};
            desc.ImageType = Key2Type[note.LaneIndex];
            desc.ImageBodyType = Key2HoldType[note.LaneIndex];
            desc.StartTime = note.StartTime;
            desc.Lane = note.LaneIndex;
            desc.Type = note.Type;
            desc.EndTime = -1;
            desc.InitialTrackPosition = m_timings->GetOffsetAt(note.StartTime);
            desc.EndTrackPosition = -1;
            desc.KeysoundIndex = note.Keysound;
            desc.StartBPM = m_timings->GetBPMAt(note.StartTime);
            desc.Volume = (int)round(note.Volume * (float)m_audioVolume);
            desc.Pan = (int)round(note.Pan * (float)m_audioVolume);

            if (note.Type == NoteType::HOLD) {
                desc.EndTime = note.EndTime;
                desc.EndTrackPosition = m_timings->GetOffsetAt(note.EndTime);
                desc.EndBPM = m_timings->GetBPMAt(note.EndTime);
            }

            if ((m_audioOffset != 0 && desc.KeysoundIndex != -1) || IsAutoSound) {
                AutoSample newSample = {};
                newSample.StartTime = desc.StartTime;
                newSample.Pan = note.Pan;
                newSample.Volume = note.Volume;
                newSample.Index = desc.KeysoundIndex;

                m_autoSamples.push_back(newSample);
                desc.KeysoundIndex = -1;
            }

            m_noteDescs.push_back(desc);
        }

        std::sort(m_noteDescs.begin(), m_noteDescs.end(), [](auto &a, auto &b) {
            if (a.StartTime != b.StartTime) {
                return a.StartTime < b.StartTime;
            } else {
                return a.EndTime < b.EndTime;
            }
        });

        std::sort(m_autoSamples.begin(), m_autoSamples.end(), [](const AutoSample &a, const AutoSample &b) {
            return a.StartTime < b.StartTime;
        });

        UpdateVirtualResolution();
        UpdateGamePosition();
        UpdateNotes();

        if (chart->m_customMeasures.size()) {
            m_timingLineManager = std::make_shared<TimingLineManager>(this, chart->m_customMeasures);
        } else {
            m_timingLineManager = std::make_shared<TimingLineManager>(this);
        }

        m_scoreManager = std::make_shared<ScoreManager>();

        m_startClock = std::chrono::system_clock::now();

        m_timingLineManager->Init();
        m_state = GameState::NotGame;
    }

    {
        if (Env::GetInt("Autoplay")) {
            std::vector<Autoplay::ReplayHitInfo> replay;

            if (info.ReplayData.size()) {
                Logs::Puts("[Gameplay] Replay mode!");

                replay = info.ReplayData;
            } else {
                Logs::Puts("[Gameplay] Autoplay mode!");

                replay = Autoplay::CreateReplay(m_currentChart);
            }

            std::sort(replay.begin(), replay.end(), [](const Autoplay::ReplayHitInfo &a, const Autoplay::ReplayHitInfo &b) {
                if (a.Time == b.Time) {
                    return a.Type < b.Type;
                }

                return a.Time < b.Time;
            });

            for (auto &hit : replay) {
                m_autoHitInfos[hit.Lane].push_back(hit);
            }

            m_autoFrames = replay;
            m_is_autoplay = true;
        }
    }

    return true;
}

bool RhythmEngine::Start()
{ // no, use update event instead
    m_currentAudioPosition -= 3000;
    m_state = GameState::Playing;

    m_startClock = std::chrono::system_clock::now();
    return true;
}

bool RhythmEngine::Stop()
{
    m_state = GameState::PosGame;
    return true;
}

bool RhythmEngine::Ready()
{
    return m_state == GameState::NotGame;
}

void RhythmEngine::UpdateNotes()
{
    for (int i = m_currentNoteIndex; i < m_noteDescs.size(); i++) {
        auto &desc = m_noteDescs[i];

        if (m_currentAudioGamePosition + (3000.0 / GetNotespeed()) > desc.StartTime || (m_currentTrackPosition - desc.InitialTrackPosition > GetPrebufferTiming())) {

            m_tracks[desc.Lane]->AddNote(desc);

            m_currentNoteIndex += 1;
        } else {
            break;
        }
    }
}

void RhythmEngine::Update(double delta)
{
    if (m_state == GameState::NotGame || m_state == GameState::PosGame)
        return;

    // Since I'm coming from Roblox, and I had no idea how to Real-Time sync the audio
    // I decided to use this method again from Roblox project I did in past.
    double last = m_currentAudioPosition;
    m_currentAudioPosition += (delta * m_rate) * 1000;

    // check difference between last and current audio position
    // if it's too big, then it means the game is lagging

    if (m_currentAudioPosition - last > 1000 * 5) {
        // assert(false); // TODO: Handle this
    }

    // m_currentAudioPosition = 104037.60425329208;

    if (m_currentAudioPosition > m_audioLength + 2500) { // Avoid game ended too early
        m_state = GameState::PosGame;
        ::printf("Audio stopped!\n");
    }

    if (static_cast<int>(m_currentAudioPosition) % 1000 == 0) {
        m_noteImageIndex = (m_noteImageIndex + 1) % m_noteMaxImageIndex;
    }

    UpdateVirtualResolution();
    UpdateGamePosition();
    UpdateNotes();

    m_timingLineManager->Update(delta);

    for (auto &it : m_tracks) {
        it->Update(delta);
    }

    // Sample event updates
    for (int i = m_currentSampleIndex; i < m_autoSamples.size(); i++) {
        auto &sample = m_autoSamples[i];
        if (m_currentAudioPosition >= sample.StartTime) {
            if (sample.StartTime - m_currentAudioPosition < 5) {
                SampleManager::Play(sample.Index, (int)round(sample.Volume * m_audioVolume), (int)round(sample.Pan * 100));
            }

            m_currentSampleIndex++;
        } else {
            break;
        }
    }

    if (m_is_autoplay) {
        auto frame = GetAutoplayAtThisFrame(m_currentAudioPosition);

        for (auto &frame : frame.KeyDowns) {
            m_tracks[frame.Lane]->OnKeyDown(frame.Time);
        }

        for (auto &frame : frame.KeyUps) {
            m_tracks[frame.Lane]->OnKeyUp(frame.Time);
        }
    }

    auto currentTime = std::chrono::system_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_startClock);
    m_PlayTime = static_cast<int>(elapsedTime.count());
}

void RhythmEngine::Draw(double delta)
{
    if (m_state == GameState::NotGame || m_state == GameState::PosGame)
        return;

    for (auto &it : m_tracks) {
        it->Render(delta);
    }
}

void RhythmEngine::DrawTimingLine(double delta)
{
    if (m_state == GameState::NotGame || m_state == GameState::PosGame)
        return;

    m_timingLineManager->Render(delta);
}

void RhythmEngine::OnKeyDown(const Inputs::State &state)
{
    if (m_state == GameState::NotGame || m_state == GameState::PosGame)
        return;

    if (state.Keyboard.Key == Inputs::Keys::F3) {
        m_scrollSpeed -= 10;
    } else if (state.Keyboard.Key == Inputs::Keys::F4) {
        m_scrollSpeed += 10;
    }

    if (!m_is_autoplay) {
        for (int i = 0; i < 7; i++) {
            auto &it = KeyMapping[i];

            if (it.key == state.Keyboard.Key) {
                it.isPressed = true;

                if (i < m_tracks.size()) {
                    m_tracks[i]->OnKeyDown(m_currentAudioPosition);
                }
            }
        }
    }
}

void RhythmEngine::OnKeyUp(const Inputs::State &state)
{
    if (m_state == GameState::NotGame || m_state == GameState::PosGame)
        return;

    if (!m_is_autoplay) {
        for (int i = 0; i < 7; i++) {
            auto &it = KeyMapping[i];

            if (it.key == state.Keyboard.Key) {
                it.isPressed = false;

                if (i < m_tracks.size()) {
                    m_tracks[i]->OnKeyUp(m_currentAudioPosition);
                }
            }
        }
    }
}

void RhythmEngine::UpdateGamePosition()
{
    m_currentAudioGamePosition = m_currentAudioPosition + m_offset;
    m_currentVisualPosition = m_currentAudioGamePosition; // * m_rate;

    while (m_currentBPMIndex + 1 < m_currentChart->m_bpms.size() && m_currentVisualPosition >= m_currentChart->m_bpms[m_currentBPMIndex + 1].StartTime) {
        m_currentBPMIndex += 1;
    }

    while (m_currentSVIndex < m_currentChart->m_svs.size() && m_currentVisualPosition >= m_currentChart->m_svs[m_currentSVIndex].StartTime) {
        m_currentSVIndex += 1;
    }

    m_currentTrackPosition = m_timings->GetOffsetAt(m_currentVisualPosition, m_currentSVIndex); // GetPositionFromOffset(m_currentVisualPosition, m_currentSVIndex);

    if (m_currentSVIndex > 0) {
        float svMultiplier = m_currentChart->m_svs[m_currentSVIndex - 1].Value;
        if (svMultiplier != m_currentSVMultiplier) {
            m_currentSVMultiplier = svMultiplier;
        }
    }

    if (m_currentBPMIndex > 0) {
        m_currentBPM = m_currentChart->m_bpms[m_currentBPMIndex - 1].Value;
    }
}

void RhythmEngine::UpdateVirtualResolution()
{
    auto rect = Graphics::NativeWindow::Get()->GetWindowSize();
    m_gameResolution = { (double)rect.Width, (double)rect.Height };

    float ratio = (float)rect.Width / (float)rect.Height;
    if (ratio >= 16.0f / 9.0f) {
        m_virtualResolution = { (double)rect.Width * ratio, (double)rect.Height };
    } else {
        m_virtualResolution = { (double)rect.Width, (double)rect.Height / ratio };
    }
}

void RhythmEngine::CreateTimingMarkers()
{
    if (m_currentChart->m_svs.size() > 0) {
        auto  &svs = m_currentChart->m_svs;
        double pos = ::round(svs[0].StartTime * m_currentChart->InitialSvMultiplier * 100);
        m_timingPositionMarkers.push_back(pos);

        for (int i = 1; i < svs.size(); i++) {
            pos += ::round((svs[i].StartTime - svs[i - 1].StartTime) * (svs[i - 1].Value * 100));

            m_timingPositionMarkers.push_back(pos);
        }
    }
}

ReplayFrameData RhythmEngine::GetAutoplayAtThisFrame(double offset)
{
    ReplayFrameData data;

    for (int i = m_autoMinIndex; i < m_autoFrames.size(); i++) {
        auto &hit = m_autoFrames[i];

        if (offset >= hit.Time) {
            if (hit.Type == Autoplay::ReplayHitType::KEY_UP) {
                data.KeyUps.push_back(hit);
            } else {
                data.KeyDowns.push_back(hit);
            }

            m_autoMinIndex++;
        } else {
            break;
        }
    }

    return data;
}

// Getters arena

double RhythmEngine::GetAudioPosition() const
{
    return 0.0;
}

const float *RhythmEngine::GetLaneSizes() const
{
    return &m_laneSize[0];
}

const float *RhythmEngine::GetLanePos() const
{
    return &m_lanePos[0];
}

void RhythmEngine::SetHitPosition(int offset)
{
    m_hitPosition = offset;
}

void RhythmEngine::SetLaneOffset(int offset)
{
    m_laneOffset = offset;
}

int RhythmEngine::GetHitPosition() const
{
    return m_hitPosition;
}

Vector2 RhythmEngine::GetResolution() const
{
    return m_gameResolution;
}

Rect RhythmEngine::GetPlayRectangle() const
{
    return m_playRectangle;
}

std::string RhythmEngine::GetTitle() const
{
    return m_title;
}

TimingBase *RhythmEngine::GetTiming() const
{
    return m_timings.get();
}

JudgeBase *RhythmEngine::GetJudge() const
{
    return m_judge.get();
}

double RhythmEngine::GetElapsedTime() const
{ // Get game frame
    return static_cast<double>(SDL_GetTicks()) / 1000.0;
}

int RhythmEngine::GetPlayTime() const
{ // Get game time
    return m_PlayTime;
}

int RhythmEngine::GetNoteImageIndex()
{
    return m_noteImageIndex;
}

int RhythmEngine::GetGuideLineIndex() const
{
    return m_guideLineIndex;
}

void RhythmEngine::SetGuideLineIndex(int idx)
{
    m_guideLineIndex = idx;
}

double RhythmEngine::GetVisualPosition() const
{
    return m_currentVisualPosition;
}

double RhythmEngine::GetGameAudioPosition() const
{
    return m_currentAudioGamePosition;
}

double RhythmEngine::GetTrackPosition() const
{
    return m_currentTrackPosition;
}

double RhythmEngine::GetPrebufferTiming() const
{
    return -300000.0 / GetNotespeed();
}

double RhythmEngine::GetNotespeed() const
{
    double speed = static_cast<double>(m_scrollSpeed);
    double scrollingFactor = 1920.0 / 1366.0;
    float  virtualRatio = (float)(m_virtualResolution.Y / m_gameResolution.Y);
    float  value = (float)((speed / 10.0) / (20.0 * m_rate) * scrollingFactor * virtualRatio);

    return value;
}

double RhythmEngine::GetBPMAt(double offset) const
{
    auto &bpms = m_currentChart->m_bpms;
    int   min = 0, max = (int)(bpms.size() - 1);

    if (max == 0) {
        return bpms[0].Value;
    }

    while (min <= max) {
        int mid = (min + max) / 2;

        bool afterMid = mid < 0 || bpms[mid].StartTime <= offset;
        bool beforeMid = mid + 1 >= bpms.size() || bpms[mid + 1].StartTime > offset;

        if (afterMid && beforeMid) {
            return bpms[mid].Value;
        } else if (afterMid) {
            max = mid - 1;
        } else {
            min = mid + 1;
        }
    }

    return bpms[0].Value;
}

double RhythmEngine::GetAudioLength() const
{
    return m_audioLength;
}

std::vector<TimingInfo> RhythmEngine::GetBPMs() const
{
    return m_currentChart->m_bpms;
}

std::vector<TimingInfo> RhythmEngine::GetSVs() const
{
    return m_currentChart->m_svs;
}

ScoreManager *RhythmEngine::GetScoreManager() const
{
    return m_scoreManager.get();
}

void RhythmEngine::ListenKeyEvent(std::function<void(TrackEvent)> callback)
{
    m_eventCallback = callback;
}

void RhythmEngine::SetKeys(Inputs::Keys *keys)
{
    for (int i = 0; i < 7; i++) {
        KeyMapping[i].key = keys[i];
    }
}

void RhythmEngine::SetSpriteBatch(
    std::shared_ptr<Graphics::SpriteBatch> note,
    std::shared_ptr<Graphics::SpriteBatch> hold,
    std::shared_ptr<Graphics::SpriteBatch> measure)
{
    m_noteSpriteBatch = note;
    m_holdSpriteBatch = hold;
    m_measureSpriteBatch = measure;
}

std::shared_ptr<Graphics::SpriteBatch> RhythmEngine::GetNoteSpriteBatch() const
{
    return m_noteSpriteBatch;
}

std::shared_ptr<Graphics::SpriteBatch> RhythmEngine::GetHoldSpriteBatch() const
{
    return m_holdSpriteBatch;
}

std::shared_ptr<Graphics::SpriteBatch> RhythmEngine::GetMeasureSpriteBatch() const
{
    return m_measureSpriteBatch;
}