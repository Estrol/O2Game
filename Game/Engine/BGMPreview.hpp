#pragma once
#include <functional>
class Chart;
class AutoSample;

class BGMPreview {
public:
	BGMPreview() = default;
	~BGMPreview();

	void Load(int index);
	void Update(double delta);
	void Play();
	void Stop();
	void Reload();

	void OnReady(std::function<void(bool)> callback);
private:
	Chart* m_currentChart = 0;

	double m_currentAudioPosition;
	double m_currentTrackPosition;
	double m_rate;
	bool OnPause;
	bool OnStarted;

	int m_currentSampleIndex = 0;
	int m_bgmIndex;
	int m_length;

	std::vector<AutoSample> m_autoSamples;
	std::function<void(bool)> m_callback;
};

