#pragma once
#include <vector>
#include "../../Engine/EstEngine.hpp"
#include "../Data/chart.hpp"
#include "GameTrack.hpp"

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

	double GetAudioPosition() const;
	double GetVisualPosition() const;
	double GetGameAudioPosition() const;
	double GetTrackPosition() const;
	double GetPrebufferTiming() const;
	double GetNotespeed() const;

private:
	void UpdateNotes();
	void UpdateGamePosition();
	void UpdateVirtualResolution();
	void CreateTimingMarkers();
	double GetPositionFromOffset(double offset);
	double GetPositionFromOffset(double offset, int index);

	double m_rate, m_offset;
	double m_currentAudioPosition;
	double m_currentVisualPosition;
	double m_currentAudioGamePosition;
	double m_currentTrackPosition;

	int m_currentSampleIndex = 0;
	int m_currentNoteIndex = 0;
	int m_currentSVIndex = 0;
	int m_scrollSpeed = 0;

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
};