#pragma once
#include "../../Engine/Keys.h"
#include <iostream>
#include "Note.hpp"

class GameTrack {
public:
	GameTrack(RhythmEngine* engine, int offset);
	~GameTrack();

	void Update(double delta);
	void Render(double delta);
	void OnKeyUp(const KeyState& key);
	void OnKeyDown(const KeyState& key);

	void AddNote(NoteInfoDesc* note);

private:
	std::vector<Note*> m_notes;
	std::vector<Note*> m_noteCaches;

	RhythmEngine* m_engine;
	int m_laneOffset;

	double m_deleteDelay;
};