#pragma once
#include <functional>
#include <thread>
#include <future>

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

	bool IsPlaying();
	bool IsReady();
	void OnReady(std::function<void(bool)> callback);
private:
	Chart* m_currentChart = 0;
	std::string m_currentFilePath = "";

	double m_currentAudioPosition;
	double m_currentTrackPosition;
	double m_rate;
	bool OnPause;
	bool OnStarted;
	bool Ready;

	int m_startOffset = 0;
	int m_currentSampleIndex = 0;
	int m_bgmIndex;
	int m_length;
	int m_currentState = 0;

	std::vector<AutoSample> m_autoSamples;
	std::function<void(bool)> m_callback;

	std::mutex* m_mutex;
	bool m_threadFinish;
};

