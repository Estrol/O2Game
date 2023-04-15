#pragma once
#include "../../Engine/Keys.h"
#include <iostream>
#include <functional>
#include "Note.hpp"

struct NoteHitInfo;

struct GameTrackEvent {
	int Lane = -1;
	
	bool State = false;
	bool IsKeyEvent = false;
	bool IsHitEvent = false;
	bool IsHitLongEvent = false;
};

class GameTrack {
public:
	GameTrack(RhythmEngine* engine, int laneIndex, int offset);
	~GameTrack();

	void Update(double delta);
	void Render(double delta);
	void OnKeyUp();
	void OnKeyDown();

	void HandleScore(NoteHitInfo info);
	void HandleHoldScore(HoldResult res);

	void AddNote(NoteInfoDesc* note);
	void ListenEvent(std::function<void(GameTrackEvent)> callback);

private:
	std::vector<Note*> m_notes;
	std::vector<Note*> m_noteCaches;

	RhythmEngine* m_engine;
	int m_laneOffset;
	int m_laneIndex;
	int m_keySound;

	double m_deleteDelay;

	Note* m_currentHold;
	bool m_onHold;

	std::function<void(GameTrackEvent)> m_callback;
};