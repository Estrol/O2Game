#include "RhythmEngine.hpp"
#include "NoteImageCacheManager.hpp"
#include "GameAudioSampleCache.hpp"
#include "../../Engine/EstEngine.hpp"
#include <filesystem>
#include <unordered_map>

struct ManiaKeyState {
	Keys key;
	bool isPressed;
};

namespace {
	std::vector<NoteImageType> Key2Type = {
		NoteImageType::WHITE,
		NoteImageType::BLUE,
		NoteImageType::WHITE,
		NoteImageType::YELLOW,
		NoteImageType::WHITE,
		NoteImageType::BLUE,
		NoteImageType::WHITE,
	};

	std::vector<NoteImageType> Key2HoldType = {
		NoteImageType::HOLD_WHITE,
		NoteImageType::HOLD_BLUE,
		NoteImageType::HOLD_WHITE,
		NoteImageType::HOLD_YELLOW,
		NoteImageType::HOLD_WHITE,
		NoteImageType::HOLD_BLUE,
		NoteImageType::HOLD_WHITE,
	};

	std::unordered_map<int, ManiaKeyState> KeyMapping = {
		{ 0, { Keys::A, false }},
		{ 1, {Keys::S, false} },
		{ 2, {Keys::D, false} },
		{ 3, {Keys::Space, false} },
		{ 4, {Keys::J, false} },
		{ 5, {Keys::K, false} },
		{ 6, {Keys::L, false} },
	};

	int trackOffset[] = { 5, 33, 55, 82, 114, 142, 164 };
}

RhythmEngine::RhythmEngine() {
	m_currentAudioGamePosition = 0;
	m_currentVisualPosition = 0;
	m_currentTrackPosition = 0;
	m_rate = 1;
	m_offset = 0;
	m_scrollSpeed = 250;

	m_timingPositionMarkers = std::vector<double>();
}

RhythmEngine::~RhythmEngine() {
	for (auto& it : m_tracks) {
		delete it;
	}
}

bool RhythmEngine::Load(Chart* chart) {
	m_currentChart = chart;

	m_autoHitIndex.clear();
	m_autoHitInfos.clear();

	for (int i = 0; i < chart->m_keyCount; i++) {
		m_tracks.push_back(new GameTrack( this, i, trackOffset[i] ));
		m_autoHitIndex[i] = 0;

		if (m_eventCallback) {
			m_tracks[i]->ListenEvent([&](int lane, bool state) {
				m_eventCallback(lane, state);
			});
		}
	}

	std::filesystem::path audioPath = chart->m_beatmapDirectory;
	audioPath /= chart->m_audio;

	if (std::filesystem::exists(audioPath)) {
		m_audioPath = audioPath.string();
	}

	for (auto& note : chart->m_notes) {
		m_notes.push_back(note);
	}

	for (auto& sample : chart->m_autoSamples) {
		m_autoSamples.push_back(sample);
	}

	auto replay = AutoReplay::CreateReplay(chart);
	std::sort(replay.begin(), replay.end(), [](const ReplayHitInfo& a, const ReplayHitInfo& b) {
		return a.Time < b.Time;
	});

	for (auto& hit : replay) {
		m_autoHitInfos[hit.Lane].push_back(hit);
	}

	std::sort(m_notes.begin(), m_notes.end(), [](const NoteInfo& a, const NoteInfo& b) {
		return a.StartTime < b.StartTime;
	});

	std::sort(m_autoSamples.begin(), m_autoSamples.end(), [](const AutoSample& a, const AutoSample& b) {
		return a.StartTime < b.StartTime;
	});

	m_beatmapOffset = chart->m_bpms[0].StartTime;

	GameAudioSampleCache::Load(chart);
	CreateTimingMarkers();
	m_state = GameState::NotGame;
	return true;
}

bool RhythmEngine::Start() {
	if (m_audioPath.size() > 0) {
		if (!AudioManager::GetInstance()->Create("main_audio", m_audioPath, &m_currentAudio)) {
			MessageBoxA(NULL, "Failed to load audio", "EstEngine Error", MB_OK);
		}
	}

	m_currentAudioPosition -= 3000;
	m_state = GameState::Playing;
	return true;
}

bool RhythmEngine::Stop() {
	return true;
}

void RhythmEngine::Update(double delta) {
	if (m_state == GameState::NotGame) return;

	m_currentAudioPosition += (delta * m_rate) * 1000;

	if (m_currentAudioPosition >= 0 && !m_started) {
		m_started = true;

		::printf("AudioStarted at pos: %.0f\n", m_currentAudioPosition);
		if (m_currentAudio) {
			m_currentAudio->Play();
		}
	}

	UpdateVirtualResolution();
	UpdateGamePosition();
	UpdateNotes();

	for (auto& it : m_tracks) {
		it->Update(delta);
	}

	for (int i = m_currentSampleIndex; i < m_autoSamples.size(); i++) {
		auto& sample = m_autoSamples[i];

		if (sample.StartTime >= m_currentAudioPosition) {
			GameAudioSampleCache::PlayEvent(i, 100);
			m_currentSampleIndex++;
		}
		else {
			break;
		}
	}

	for (int i = 0; i < 7; i++) {
		int& index = m_autoHitIndex[i];

		if (index < m_autoHitInfos[i].size() 
			&& m_currentAudioGamePosition >= m_autoHitInfos[i][index].Time) {
			
			auto& info = m_autoHitInfos[i][index];

			if (info.Type == ReplayHitType::KEY_DOWN) {
				m_tracks[i]->OnKeyDown();
			}
			else {
				m_tracks[i]->OnKeyUp();
			}

			index++;
		}
	}
}

void RhythmEngine::Render(double delta) {
	if (m_state == GameState::NotGame) return;

	RECT playArea = { 0, 0, 198, 480 };

	auto batch = Renderer::GetInstance()->GetSpriteBatch(1);
	auto states = Renderer::GetInstance()->GetStates();
	auto context = Renderer::GetInstance()->GetImmediateContext();
	auto rasterizerState = Renderer::GetInstance()->GetRasterizerState();

	batch->Begin(
		DirectX::SpriteSortMode_Deferred,
		states->NonPremultiplied(),
		nullptr,
		nullptr,
		rasterizerState,
		[&] {
			CD3D11_RECT rect(playArea);
			context->RSSetScissorRects(1, &rect);
		}
	);

	for (auto& it : m_tracks) {
		it->Render(delta);
	}

	batch->End();
}

void RhythmEngine::OnKeyDown(const KeyState& state) {
	if (m_state == GameState::NotGame) return;

	if (state.key == Keys::O) {
		m_scrollSpeed -= 10;
	}
	else if (state.key == Keys::P) {
		m_scrollSpeed += 10;
	}

	for (auto& key : KeyMapping) {
		if (key.second.key == state.key) {
			key.second.isPressed = true;

			m_tracks[key.first]->OnKeyDown();
		}
	}
}

void RhythmEngine::OnKeyUp(const KeyState& state) {
	if (m_state == GameState::NotGame) return;

	for (auto& key : KeyMapping) {
		if (key.second.key == state.key) {
			key.second.isPressed = false;

			m_tracks[key.first]->OnKeyUp();
		}
	}
}

void RhythmEngine::ListenKeyEvent(std::function<void(int,bool)> callback) {
	m_eventCallback = callback;
}

double RhythmEngine::GetAudioPosition() const {
	return m_currentAudioPosition;
}

double RhythmEngine::GetVisualPosition() const {
	return m_currentVisualPosition;
}

double RhythmEngine::GetGameAudioPosition() const {
	return m_currentAudioGamePosition;
}

double RhythmEngine::GetTrackPosition() const {
	return m_currentTrackPosition;
}

double RhythmEngine::GetPrebufferTiming() const {
	return -300000.0 / GetNotespeed();
}

double RhythmEngine::GetNotespeed() const {
	double speed = static_cast<double>(m_scrollSpeed);
	double scrollingFactor = 1920.0 / 1366.0;
	float virtualRatio = m_virtualResolution.Y / 600.0;

	return (speed / 10.0) / (20.0 / m_rate) * scrollingFactor * virtualRatio;
}

void RhythmEngine::UpdateNotes() {
	for (int i = m_currentNoteIndex; i < m_notes.size(); i++) {
		auto& note = m_notes[i];
		double startTime = GetPositionFromOffset(note.StartTime);
		
		if (m_currentAudioGamePosition + (3000.0 / GetNotespeed()) > note.StartTime
			|| (m_currentTrackPosition - startTime > GetPrebufferTiming())) {
			
			NoteInfoDesc desc = {};
			desc.ImageType = Key2Type[note.LaneIndex];
			desc.ImageBodyType = Key2HoldType[note.LaneIndex];
			desc.StartTime = note.StartTime;
			desc.Lane = note.LaneIndex;
			desc.Type = note.Type;
			desc.EndTime = -1;
			desc.InitialTrackPosition = startTime;

			if (note.Type == NoteType::HOLD) {
				desc.EndTime = note.EndTime;
				desc.EndTrackPosition = GetPositionFromOffset(note.EndTime);
			}
			
			m_tracks[note.LaneIndex]->AddNote(&desc);

			m_currentNoteIndex += 1;
		}
		else {
			break;
		}
	}
}

void RhythmEngine::UpdateGamePosition() {
	m_currentAudioGamePosition = m_currentAudioPosition + m_offset;
	m_currentVisualPosition = m_currentAudioGamePosition * m_rate;

	while (m_currentSVIndex < m_currentChart->m_svs.size() && m_currentVisualPosition >= m_currentChart->m_svs[m_currentSVIndex].StartTime) {
		m_currentSVIndex += 1;
	}

	m_currentTrackPosition = GetPositionFromOffset(m_currentVisualPosition, m_currentSVIndex);
}

void RhythmEngine::UpdateVirtualResolution() {
	double width = Window::GetInstance()->GetHeight();
	double height = Window::GetInstance()->GetHeight();

	float ratio = (float)width / (float)height;
	if (ratio >= 16.0f / 9.0f) {
		m_virtualResolution = { width * ratio, height };
	}
	else {
		m_virtualResolution = { width, height / ratio };
	}
}

void RhythmEngine::CreateTimingMarkers() {
	if (m_currentChart->m_svs.size() > 0) {
		auto& svs = m_currentChart->m_svs;
		double pos = std::round(svs[0].StartTime * m_currentChart->InitialSvMultiplier * 100);
		m_timingPositionMarkers.push_back(pos);

		for (int i = 1; i < svs.size(); i++) {
			pos += std::round(svs[i].StartTime - svs[i - 1].StartTime) * (svs[i - 1].Value * 100);

			m_timingPositionMarkers.push_back(pos);
		}
	}
}

double RhythmEngine::GetPositionFromOffset(double offset) {
	int index;

	for (index = 0; index < m_currentChart->m_svs.size(); index++) {
		if (offset < m_currentChart->m_svs[index].StartTime) {
			break;
		}
	}

	return GetPositionFromOffset(offset, index);
}

double RhythmEngine::GetPositionFromOffset(double offset, int index) {
	if (index == 0) {
		return offset * m_currentChart->InitialSvMultiplier * 100;
	}

	index -= 1;

	double pos = m_timingPositionMarkers[index];
	pos += (offset - m_currentChart->m_svs[index].StartTime) * (m_currentChart->m_svs[index].Value * 100);

	return pos;
}
