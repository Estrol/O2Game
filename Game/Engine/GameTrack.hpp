#pragma once
#include "../../Engine/Keys.h"
#include <iostream>
#include <functional>
#include "Note.hpp"

class GameTrack {
public:
	GameTrack(RhythmEngine* engine, int laneIndex, int offset);
	~GameTrack();

	void Update(double delta);
	void Render(double delta);
	void OnKeyUp();
	void OnKeyDown();

	void AddNote(NoteInfoDesc* note);
	void ListenEvent(std::function<void(int, bool)> callback);

private:
	std::vector<Note*> m_notes;
	std::vector<Note*> m_noteCaches;

	RhythmEngine* m_engine;
	int m_laneOffset;
	int m_laneIndex;

	double m_deleteDelay;

	std::function<void(int, bool)> m_callback;
};