#pragma once
class Chart;
class AudioSampleChannel;

namespace GameAudioSampleCache {
	void Load(Chart* chart);

	void Play(int index, int volume);
	void Stop(int index);
	void SetRate(double rate);
	void ResumeAll();
	void PauseAll();
	void StopAll();

	void Dispose();
}