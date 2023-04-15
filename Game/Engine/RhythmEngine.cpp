#include "RhythmEngine.hpp"
#include "NoteImageCacheManager.hpp"
#include "GameAudioSampleCache.hpp"
#include "../../Engine/EstEngine.hpp"
#include <filesystem>
#include <unordered_map>
#include "NoteResult.hpp"

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
	m_rate = 1.25;
	m_offset = 0;
	m_scrollSpeed = 180;

	m_timingPositionMarkers = std::vector<double>();
}

RhythmEngine::~RhythmEngine() {
	Release();
}

bool RhythmEngine::Load(Chart* chart) {
	m_state = GameState::PreParing;
	m_currentChart = chart;

	m_autoHitIndex.clear();
	m_autoHitInfos.clear();

	int startX = 4;
	int currentX = startX;
	for (int i = 0; i < chart->m_keyCount; i++) {
		m_tracks.push_back(new GameTrack( this, i, currentX ));
		m_autoHitIndex[i] = 0;

		if (m_eventCallback) {
			m_tracks[i]->ListenEvent([&](GameTrackEvent e) {
				m_eventCallback(e);
			});
		}

		int size = GameNoteResource::GetNoteTexture(Key2Type[i])->TextureRect.right;
		
		m_lanePos[i] = currentX;
		m_laneSize[i] = size;
		currentX += size;
	}

	m_playRectangle = { 0, 0, currentX, 480 };

	std::filesystem::path audioPath = chart->m_beatmapDirectory;
	audioPath /= chart->m_audio;

	if (std::filesystem::exists(audioPath) && !audioPath.string().ends_with("\\")) {
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
		if (a.StartTime != b.StartTime) {
			return a.StartTime < b.StartTime;
		}
		else {
			return a.EndTime < b.EndTime;
		}
	});

	std::sort(m_autoSamples.begin(), m_autoSamples.end(), [](const AutoSample& a, const AutoSample& b) {
		return a.StartTime < b.StartTime;
	});

	m_beatmapOffset = chart->m_bpms[0].StartTime;
	m_audioLength = m_notes[m_notes.size() - 1].EndTime != 0 ? m_notes[m_notes.size() - 1].EndTime : m_notes[m_notes.size() - 1].StartTime; //chart->GetLength();
	m_baseBPM = chart->BaseBPM;
	m_currentBPM = m_baseBPM;
	m_currentSVMultiplier = chart->InitialSvMultiplier;

	GameAudioSampleCache::Load(chart);
	
	CreateTimingMarkers();
	UpdateVirtualResolution();
	UpdateGamePosition();
	UpdateNotes();

	m_timingLineManager = new TimingLineManager(this);
	m_scoreManager = new ScoreManager();

	m_state = GameState::NotGame;
	return true;
}

bool RhythmEngine::Start() {
	std::thread([&] {
		std::cout << "Starting Engine 1" << std::endl;
		if (m_audioPath.size() > 0) {
			AudioManager::GetInstance()->Create("main_audio", m_audioPath, &m_currentAudio);
		}

		m_currentAudioPosition -= 3000;
		m_state = GameState::Playing;

		GameAudioSampleCache::SetRate(m_rate);
		std::cout << "Starting Engine 2" << std::endl;
	}).detach();
	
	return true;
}

bool RhythmEngine::Stop() {
	return true;
}

bool RhythmEngine::Ready() {
	return m_state == GameState::NotGame;
}

void RhythmEngine::Update(double delta) {
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;

	// Since I'm coming from Roblox, and I had no idea how to Real-Time sync the audio
	// I decided to use this method again from Roblox project I did in past.
	m_currentAudioPosition += (delta * m_rate) * 1000;

	if (m_currentAudioPosition >= 0 && !m_started) {
		m_started = true;

		::printf("AudioStarted at pos: %.0f\n", m_currentAudioPosition);
		if (m_currentAudio) {
			m_currentAudio->SetRate(m_rate);
			m_currentAudio->SetVolume(100);
			m_currentAudio->Play();
		}
	}

	if (m_currentAudioPosition > m_audioLength + 2500) {
		m_state = GameState::PosGame;
		::printf("AudioStopped!\n");
	}

	UpdateVirtualResolution();
	UpdateGamePosition();
	UpdateNotes();

	m_timingLineManager->Update(delta);

	for (auto& it : m_tracks) {
		it->Update(delta);
	}

	// Sample event updates
	for (int i = m_currentSampleIndex; i < m_autoSamples.size(); i++) {
		auto& sample = m_autoSamples[i];

		if (m_currentVisualPosition >= sample.StartTime) {
			GameAudioSampleCache::Play(sample.Index, 50);
			m_currentSampleIndex++;
		}
		else {
			break;
		}
	}

	// Autoplay updates
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
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;

	RECT playArea = { 0, 0, 198, 480 };

	auto batch = Renderer::GetInstance()->GetSpriteBatch(1);
	auto states = Renderer::GetInstance()->GetStates();
	auto context = Renderer::GetInstance()->GetImmediateContext();
	auto rasterizerState = Renderer::GetInstance()->GetRasterizerState();

	batch->Begin(
		DirectX::SpriteSortMode_Deferred,
		states->NonPremultiplied(),
		states->PointWrap(),
		nullptr,
		rasterizerState,
		[&] {
			CD3D11_RECT rect(playArea);
			context->RSSetScissorRects(1, &rect);
		}
	);

	m_timingLineManager->Render(delta);

	for (auto& it : m_tracks) {
		it->Render(delta);
	}

	batch->End();
}

void RhythmEngine::OnKeyDown(const KeyState& state) {
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;

	if (state.key == Keys::O) {
		m_scrollSpeed -= 10;
	}
	else if (state.key == Keys::P) {
		m_scrollSpeed += 10;
	}

	for (auto& key : KeyMapping) {
		if (key.second.key == state.key) {
			key.second.isPressed = true;

			if (key.first < m_tracks.size()) {
				m_tracks[key.first]->OnKeyDown();
			}
		}
	}
}

void RhythmEngine::OnKeyUp(const KeyState& state) {
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;

	for (auto& key : KeyMapping) {
		if (key.second.key == state.key) {
			key.second.isPressed = false;

			if (key.first < m_tracks.size()) {
				m_tracks[key.first]->OnKeyUp();
			}
		}
	}
}

void RhythmEngine::ListenKeyEvent(std::function<void(GameTrackEvent)> callback) {
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
	double scrollingFactor = 800.0 / 480.0;
	float virtualRatio = m_virtualResolution.Y / 600.0;

	return (speed / 10.0) / (20.0 * m_rate) * scrollingFactor * virtualRatio;
}

bool CompareBeatOffset(const TimingInfo& a, const TimingInfo& b) {
	return a.StartTime <= b.StartTime;
}

double RhythmEngine::GetBeat(double offset) const {
	TimingInfo toSearch = {};
	toSearch.StartTime = offset;

	auto& bpms = m_currentChart->m_bpms;

	auto it = std::lower_bound(bpms.begin(), bpms.end(), toSearch, CompareBeatOffset);
	if (it == bpms.end()) {
		it = bpms.begin();
	}
	
	double beat = it->Beat;
	double bpm = it->Value;
	double startTime = it->StartTime;

	return beat + (offset - startTime) * (bpm / 60000.0);
}

double RhythmEngine::GetSongRate() const {
	return m_rate;
}

int RhythmEngine::GetAudioLength() const {
	return m_audioLength;
}

GameState RhythmEngine::GetState() const {
	return m_state;
}

ScoreManager* RhythmEngine::GetScoreManager() const {
	return m_scoreManager;
}

std::vector<double> RhythmEngine::GetTimingWindow() {
	float ratio = std::clamp(2.0f - m_currentSVMultiplier, 0.1f, 2.0f);
	
	return { kNoteCoolHitWindowMax * ratio, kNoteGoodHitWindowMax * ratio, kNoteBadHitWindowMax * ratio, kNoteEarlyMissWindowMin * ratio };
}

std::vector<TimingInfo> RhythmEngine::GetBPMs() const {
	return m_currentChart->m_bpms;
}

std::vector<TimingInfo> RhythmEngine::GetSVs() const {
	return m_currentChart->m_svs;
}

void RhythmEngine::UpdateNotes() {
	for (int i = m_currentNoteIndex; i < m_notes.size(); i++) {
		auto& note = m_notes[i];
		double startTime = GetPositionFromOffset(note.StartTime);
		double endTime = 0;
		if (note.EndTime != -1) {
			endTime = GetPositionFromOffset(note.EndTime);
		}
		
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
			desc.EndTrackPosition = -1;
			desc.KeysoundIndex = note.Keysound;

			if (note.Type == NoteType::HOLD) {
				desc.EndTime = note.EndTime;
				desc.EndTrackPosition = endTime;
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
	m_currentVisualPosition = m_currentAudioGamePosition;// * m_rate;

	while (m_currentBPMIndex + 1 < m_currentChart->m_bpms.size() && m_currentVisualPosition >= m_currentChart->m_bpms[m_currentBPMIndex + 1].StartTime) {
		m_currentBPMIndex += 1;
	}

	while (m_currentSVIndex < m_currentChart->m_svs.size() && m_currentVisualPosition >= m_currentChart->m_svs[m_currentSVIndex].StartTime) {
		m_currentSVIndex += 1;
	}

	m_currentTrackPosition = GetPositionFromOffset(m_currentVisualPosition, m_currentSVIndex);

	if (m_currentSVIndex > 0) {
		float svMultiplier = m_currentChart->m_svs[m_currentSVIndex - 1].Value;
		if (svMultiplier != m_currentSVMultiplier) {
			m_currentSVMultiplier = svMultiplier;

			std::cout << "SV Multiplier changed to: " << svMultiplier << "\n";
		}
	}
}

void RhythmEngine::UpdateVirtualResolution() {
	double width = Window::GetInstance()->GetBufferHeight();
	double height = Window::GetInstance()->GetBufferHeight();

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
			pos += std::round((svs[i].StartTime - svs[i - 1].StartTime) * (svs[i - 1].Value * 100));

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

int* RhythmEngine::GetLaneSizes() const {
	return (int*)&m_laneSize;
}

int* RhythmEngine::GetLanePos() const {
	return (int*)&m_lanePos;
}

void RhythmEngine::Release() {
	for (int i = 0; i < m_tracks.size(); i++) {
		delete m_tracks[i];
	}

	delete m_timingLineManager;
}
