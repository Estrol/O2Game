#pragma once
#include <vector>
#include "../../Engine/EstEngine.hpp"
#include "../Data/Chart.hpp"
#include "../Data/AutoReplay.hpp"
#include "GameTrack.hpp"
#include "TimingLineManager.hpp"

enum class GameState {
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
	bool Start();
	bool Stop();

	void Update(double delta);
	void Render(double delta);

	void OnKeyDown(const KeyState& key);
	void OnKeyUp(const KeyState& key);

	void ListenKeyEvent(std::function<void(int,bool)> callback);

	double GetAudioPosition() const;
	double GetVisualPosition() const;
	double GetGameAudioPosition() const;
	double GetTrackPosition() const;
	double GetPrebufferTiming() const;
	double GetNotespeed() const;
	int GetAudioLength() const;

	double GetPositionFromOffset(double offset);
	double GetPositionFromOffset(double offset, int index);

	std::vector<TimingInfo> GetBPMs() const;
	std::vector<TimingInfo> GetSVs() const;

private:
	void UpdateNotes();
	void UpdateGamePosition();
	void UpdateVirtualResolution();
	void CreateTimingMarkers();

	void Release();

	double m_rate, m_offset, m_beatmapOffset;
	double m_currentAudioPosition;
	double m_currentVisualPosition;
	double m_currentAudioGamePosition;
	double m_currentTrackPosition;

	int m_currentSampleIndex = 0;
	int m_currentNoteIndex = 0;
	int m_currentSVIndex = 0;
	int m_scrollSpeed = 0;
	int m_audioLength = 0;

	bool m_started = false;
	GameState m_state = GameState::NotGame;

	std::string m_audioPath = "";
	Audio* m_currentAudio;
	Chart* m_currentChart;
	Vector2 m_virtualResolution;
	std::vector<double> m_timingPositionMarkers;
	std::vector<GameTrack*> m_tracks;
	std::vector<NoteInfo> m_notes;
	std::vector<AutoSample> m_autoSamples;
	std::unordered_map<int, int> m_autoHitIndex;
	std::unordered_map<int, std::vector<ReplayHitInfo>> m_autoHitInfos;

	TimingLineManager* m_timingLineManager;
	std::function<void(int, bool)> m_eventCallback;
};