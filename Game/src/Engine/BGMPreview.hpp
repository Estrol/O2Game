#pragma once
#include <functional>
#include <future>
#include <thread>

class Chart;
struct AutoSample;

class BGMPreview
{
public:
    BGMPreview() = default;
    ~BGMPreview();

    void Load();
    void Update(double delta);
    void Play();
    void Stop();
    void Reload();

    bool IsPlaying();
    bool IsReady();
    void OnReady(std::function<void(bool)> callback);

private:
    Chart      *m_currentChart = 0;
    std::string m_currentFilePath = "";

    double m_currentAudioPosition;
    double m_currentTrackPosition;
    double m_rate;
    bool   OnPause;
    bool   OnStarted;
    bool   Ready;

    double m_startOffset = 0;
    int    m_currentSampleIndex = 0;
    int    m_bgmIndex;
    double m_length;
    int    m_currentState = 0;

    std::vector<AutoSample>   m_autoSamples;
    std::function<void(bool)> m_callback;

    std::mutex *m_mutex;
    bool        m_threadFinish;
};
