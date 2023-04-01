#pragma once

#include "TimingLine.hpp"
#include "../Resources/iterable_queue.hpp"
#include <vector>

class RhythmEngine;

class TimingLineManager {
public:
	TimingLineManager(RhythmEngine* engine);
	~TimingLineManager();

	void Update(double delta);
	void Render(double delta);

private:
	RhythmEngine* m_engine;
	iterable_queue<TimingLineDesc> m_timingInfos;
	iterable_queue<TimingLine*> m_timingLines;
};