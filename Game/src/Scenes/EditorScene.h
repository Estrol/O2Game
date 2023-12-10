#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "Audio/AudioSample.h"
#include "Imgui/imgui.h"
#include "Scene.h"

#include "../Data/OJN.h"

struct INote;
struct O2Sample;
struct O2Note;
struct LineInfo;

class EditorScene : public Scene
{
public:
    EditorScene();

    void Update(double delta) override;
    void Render(double delta) override;

    bool Attach() override;
    bool Detach() override;

private:
    void PlaySample(int idx);
    void StopSample();

    void LoadDifficulty(int idx);

    std::vector<INote>    m_notes;
    std::vector<O2Sample> m_samples;

    std::vector<std::pair<double, double>>        m_bpms;
    std::unordered_map<int, AudioSample *>        m_audio_sample;
    std::unordered_map<int, AudioSampleChannel *> m_tracked_audio_sample;

    std::vector<LineInfo> m_lines;
    std::vector<LineInfo> m_majorLines;

    int m_measureGridSize = 16, m_measureGridSeparator = 4;
    int m_currentDifficulty = 0;

    float  m_currentTime;
    float  m_currentNotespeed;
    double m_currentMultiplier = 1;
    float  m_laneWidth = 40.25;

    ImVec2 m_oldBufferSize;
    ImVec2 m_lastMousePos;
    bool   m_isMouseClickSlider;

    std::unique_ptr<O2::OJN> m_ojn;

    bool m_ready;
    bool m_autoscroll;
    bool m_exit;
};
