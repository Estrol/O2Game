#pragma once
class Chart;
class AudioSampleChannel;

namespace GameAudioSampleCache {
	void Load(Chart* chart);

	AudioSampleChannel* Play(int index, int volume);
	AudioSampleChannel* PlayEvent(int index, int volume);

	void ResumeAll();
	void PauseAll();
	void StopAll();

	void Dispose();
}