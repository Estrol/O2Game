#pragma once
#include <vector>
#include "Rendering/WindowsTypes.h"
#include "Texture/Vector2.h"
#include "../Data/Chart.hpp"
#include "../Data/AutoReplay.hpp"
#include "GameTrack.hpp"
#include "TimingLineManager.hpp"
#include "ScoreManager.hpp"
#include "Timing/TimingBase.h"
#include "Judgements/JudgeBase.h"

enum class GameState {
	PreParing,
	NotGame,
	PreGame,
	Playing,
	PosGame
};

class RhythmEngine {
public:
	RhythmEngine();
	~RhythmEngine();

	bool Load(Chart* chart);
	void SetKeys(Keys* keys);

	bool Start();
	bool Stop();
	bool Ready();

	void Update(double delta);
	void Render(double delta);
	void Input(double delta);

	void OnKeyDown(const KeyState& key);
	void OnKeyUp(const KeyState& key);

	void ListenKeyEvent(std::function<void(GameTrackEvent)> callback);

	double GetAudioPosition() const;
	double GetVisualPosition() const;
	double GetGameAudioPosition() const;
	double GetTrackPosition() const;
	double GetPrebufferTiming() const;
	double GetNotespeed() const;
	double GetBPMAt(double offset) const;
	double GetCurrentBPM() const;
	double GetSongRate() const;
	double GetAudioLength() const;
	int GetGameVolume() const;

	int* GetLaneSizes() const;
	int* GetLanePos() const;

	void SetHitPosition(int offset);
	void SetLaneOffset(int offset);
	int GetHitPosition() const;
	Vector2 GetResolution() const;
	Rect GetPlayRectangle() const;
	std::u8string GetTitle() const;
	
	GameState GetState() const;
	TimingBase* GetTiming() const;
	JudgeBase* GetJudge() const;
	ScoreManager* GetScoreManager() const;
	std::vector<TimingInfo> GetBPMs() const;
	std::vector<TimingInfo> GetSVs() const;

	double GetElapsedTime() const;
	int GetPlayTime() const;
	int GetNoteImageIndex();

	int GetGuideLineIndex() const;
	void SetGuideLineIndex(int idx);

private:
	void UpdateNotes();
	void UpdateGamePosition();
	void UpdateVirtualResolution();
	void CreateTimingMarkers();
	std::vector<ReplayHitInfo> GetAutoplayAtThisFrame(double offset);

	void Release();

	double m_rate = 0.0, m_offset = 0.0, m_beatmapOffset = 0.0;
	double m_currentAudioPosition = 0.0;
	double m_currentVisualPosition = 0.0;
	double m_currentAudioGamePosition = 0.0;
	double m_currentTrackPosition = 0.0;
	float m_baseBPM, m_currentBPM = 0.0;
	float m_currentSVMultiplier = 0.0;

	int m_currentSampleIndex = 0;
	int m_currentNoteIndex = 0;
	int m_currentBPMIndex = 0;
	int m_currentSVIndex = 0;
	int m_scrollSpeed = 0;
	double m_audioLength = 0;
	int m_hitPosition = 0;
	int m_laneOffset = 0;
	int m_audioVolume = 100;
	int m_audioOffset = 0;

	int m_noteImageIndex = 0;
	int m_noteMaxImageIndex = 0;

	int m_guideLineIndex = 0;
	int m_autoMinIndex = 0;

	bool m_started = false;
	bool m_is_autoplay = false;

	GameState m_state = GameState::NotGame;
	std::u8string m_title;

	Rect m_playRectangle;
	int m_laneSize[7];
	int m_lanePos[7];

	std::filesystem::path m_audioPath = "";
	Chart* m_currentChart;
	Vector2 m_virtualResolution = { 0, 0 };
	Vector2 m_gameResolution = { 0, 0 };
	std::vector<double> m_timingPositionMarkers;
	std::vector<GameTrack*> m_tracks;
	std::vector<NoteInfoDesc> m_noteDescs;
	std::vector<AutoSample> m_autoSamples;
	std::unordered_map<int, int> m_autoHitIndex;
	std::unordered_map<int, std::vector<ReplayHitInfo>> m_autoHitInfos;
	std::vector<ReplayHitInfo> m_autoFrames;

	/* clock system */
	int m_PlayTime = 0;
	std::chrono::system_clock::time_point m_startClock;

	TimingBase* m_timings;
	JudgeBase* m_judge;
	ScoreManager* m_scoreManager;
	TimingLineManager* m_timingLineManager;
	std::function<void(GameTrackEvent)> m_eventCallback;
};