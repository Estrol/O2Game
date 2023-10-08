#pragma once
#include <vector>

class Chart;
class AudioSampleChannel;

namespace GameAudioSampleCache {
	void Load(Chart* chart, bool pitch);
	void Load(Chart* chart, bool pitch, bool force);

	bool IsEmpty();

	void Play(int index, int volume = 100, int pan = 0);
	void Stop(int index);
	void SetRate(double rate);
	double SetRate();
	void ResumeAll();
	void PauseAll();
	void StopAll();

	std::vector<float> QueryMixerData();
	
	void Dispose();
}