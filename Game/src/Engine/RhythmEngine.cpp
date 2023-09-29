#include "RhythmEngine.hpp"
#include <unordered_map>
#include <filesystem>
#include <numeric>

#include "../EnvironmentSetup.hpp"
#include "Configuration.h"
#include "Rendering/Window.h"

#include "NoteImageCacheManager.hpp"
#include "GameAudioSampleCache.hpp"

#include "Timing/StaticTiming.h"
#include "Timing/VelocityTiming.h"

#include "Judgements/BeatBasedJudge.h"
#include "Judgements/MsBasedJudge.h"


#include <chrono>
#include <codecvt>

#define MAX_BUFFER_TXT_SIZE 256

struct ManiaKeyState {
	Keys key;
	bool isPressed;
};

namespace {
	std::vector<NoteImageType> Key2Type = {
		NoteImageType::LANE_1,
		NoteImageType::LANE_2,
		NoteImageType::LANE_3,
		NoteImageType::LANE_4,
		NoteImageType::LANE_5,
		NoteImageType::LANE_6,
		NoteImageType::LANE_7,
	};

	std::vector<NoteImageType> Key2HoldType = {
		NoteImageType::HOLD_LANE_1,
		NoteImageType::HOLD_LANE_2,
		NoteImageType::HOLD_LANE_3,
		NoteImageType::HOLD_LANE_4,
		NoteImageType::HOLD_LANE_5,
		NoteImageType::HOLD_LANE_6,
		NoteImageType::HOLD_LANE_7,
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
	m_scrollSpeed = 180;

	m_timingPositionMarkers = std::vector<double>();
}

RhythmEngine::~RhythmEngine() {
	Release();
}

bool RhythmEngine::Load(Chart* chart) {
	GameNoteResource::Load();
	
	m_state = GameState::PreParing;
	m_currentChart = chart;

	m_autoHitIndex.clear();
	m_autoHitInfos.clear();

	// default is 99
	m_noteMaxImageIndex = 99;

	int currentX = m_laneOffset;
	for (int i = 0; i < 7; i++) {
		m_tracks.push_back(new GameTrack( this, i, currentX ));
		m_autoHitIndex[i] = 0;

		if (m_eventCallback) {
			m_tracks[i]->ListenEvent([&](GameTrackEvent e) {
				m_eventCallback(e);
			});
		}

		auto noteTex = GameNoteResource::GetNoteTexture(Key2Type[i]);

		int size = noteTex->TextureRect.right;
		m_noteMaxImageIndex = (std::min)(noteTex->MaxFrames, m_noteMaxImageIndex);
		
		m_lanePos[i] = currentX;
		m_laneSize[i] = size;
		currentX += size;
	}

	currentX = std::accumulate(m_laneSize, m_laneSize + 7, 0);
	m_playRectangle = { m_laneOffset, 0, m_laneOffset + currentX, m_hitPosition };

	std::filesystem::path audioPath = chart->m_beatmapDirectory;
	audioPath /= chart->m_audio;

	bool isSV = true;
	if (EnvironmentSetup::GetInt("Mirror")) {
		chart->ApplyMod(Mod::MIRROR);
	}
	else if (EnvironmentSetup::GetInt("Random")) {
		chart->ApplyMod(Mod::RANDOM);
	}
	else if (EnvironmentSetup::GetInt("Rearrange")) {
		void* lane_data = EnvironmentSetup::GetObj("LaneData");

		chart->ApplyMod(Mod::REARRANGE, lane_data);
	}
	else if (EnvironmentSetup::GetInt("NoSV")) {
		isSV = true;
	}

	if (isSV) {
		m_timings = new VelocityTiming(chart->m_bpms, chart->m_svs, chart->InitialSvMultiplier);
	} else {
		m_timings = new StaticTiming(chart->m_bpms, chart->m_svs, chart->InitialSvMultiplier);
	}

	m_judge = new BeatBasedJudge(this);

	if (std::filesystem::exists(audioPath) && audioPath.has_extension()) {
		m_audioPath = audioPath;
	}

	for (auto& sample : chart->m_autoSamples) {
		m_autoSamples.push_back(sample);
	}

	if (EnvironmentSetup::GetInt("Autoplay") == 1) {
		std::cout << "AutoPlay enabled!" << std::endl;
		auto replay = Autoplay::CreateReplay(chart);
		std::sort(replay.begin(), replay.end(), [](const Autoplay::ReplayHitInfo& a, const Autoplay::ReplayHitInfo& b) {
			if (a.Time == b.Time) {
				return a.Type < b.Type;
			}
			
			return a.Time < b.Time;
		});

		for (auto& hit : replay) {
			m_autoHitInfos[hit.Lane].push_back(hit);
		}

		m_autoFrames = replay;
		m_is_autoplay = true;
	}

	auto audioVolume = Configuration::Load("Game", "AudioVolume");
	if (audioVolume.size() > 0) {
		try {
			m_audioVolume = std::stoi(audioVolume);
		}
		catch (std::invalid_argument) {
			std::cout << "Game.ini::AudioVolume invalid volume: " << audioVolume << " reverting to 100 value" << std::endl;
			m_audioVolume = 100;
		}
	}

	auto audioOffset = Configuration::Load("Game", "AudioOffset");
	if (audioOffset.size() > 0) {
		try {
			m_audioOffset = std::stoi(audioOffset);
		}

		catch (std::invalid_argument) {
			std::cout << "Game.ini::AudioOffset invalid offset: " << audioOffset << " reverting to 0 value" << std::endl;
			m_audioOffset = 0;
		}
	}

	auto autoSound = Configuration::Load("Game", "AutoSound");
	bool IsAutoSound = false;
	if (autoSound.size() > 0) {
		try {
			IsAutoSound = std::stoi(autoSound) == 1;
		}

		catch (std::invalid_argument) {
			std::cout << "Game.ini::AutoSound invalid value: " << autoSound << " reverting to 0 value" << std::endl;
			IsAutoSound = false;
		}
	}

	auto noteSpeed = Configuration::Load("Gameplay", "Notespeed");
	if (noteSpeed.size() > 0) {
		try {
			m_scrollSpeed = std::stoi(noteSpeed);
		} catch (std::invalid_argument) {
			std::cout << "Game.ini::NoteSpeed invalid value: " << noteSpeed << " reverting to 210 value" << std::endl;
		}
	}

	if (EnvironmentSetup::Get("SongRate").size() > 0) {
		m_rate = std::stod(EnvironmentSetup::Get("SongRate").c_str());
		m_rate = std::clamp(m_rate, 0.5, 2.0);
	}

	m_title = chart->m_title;
	char buffer[MAX_BUFFER_TXT_SIZE];
	sprintf(buffer, "Lv.%d %s", chart->m_level, (const char*)chart->m_title.c_str());

	m_title = std::u8string(buffer, buffer + strlen(buffer));
	if (m_rate != 1.0) {
		memset(buffer, 0, MAX_BUFFER_TXT_SIZE);
		sprintf(buffer, "[%.2fx] %s", m_rate, (const char*)m_title.c_str());

		m_title = std::u8string(buffer, buffer + strlen(buffer));
	}

	m_beatmapOffset = chart->m_bpms[0].StartTime;
	m_audioLength = chart->GetLength();
	m_baseBPM = chart->BaseBPM;
	m_currentBPM = m_baseBPM;
	m_currentSVMultiplier = chart->InitialSvMultiplier;

	bool isPitch = Configuration::Load("Game", "AudioPitch") == "1";
	GameAudioSampleCache::SetRate(m_rate);
	GameAudioSampleCache::Load(chart, isPitch);
	
	CreateTimingMarkers();
	UpdateVirtualResolution();

	for (auto& note : chart->m_notes) {
		NoteInfoDesc desc = {};
		desc.ImageType = Key2Type[note.LaneIndex];
		desc.ImageBodyType = Key2HoldType[note.LaneIndex];
		desc.StartTime = note.StartTime;
		desc.Lane = note.LaneIndex;
		desc.Type = note.Type;
		desc.EndTime = -1;
		desc.InitialTrackPosition = m_timings->GetOffsetAt(note.StartTime);
		desc.EndTrackPosition = -1;
		desc.KeysoundIndex = note.Keysound;
		desc.StartBPM = m_timings->GetBPMAt(note.StartTime);
		desc.Volume = (int)round(note.Volume * (float)m_audioVolume);
		desc.Pan = (int)round(note.Pan * (float)m_audioVolume);

		if (note.Type == NoteType::HOLD) {
			desc.EndTime = note.EndTime;
			desc.EndTrackPosition = m_timings->GetOffsetAt(note.EndTime);
			desc.EndBPM = m_timings->GetBPMAt(note.EndTime);
		}

		if ((m_audioOffset != 0 && desc.KeysoundIndex != -1) || IsAutoSound) {
			AutoSample newSample = {};
			newSample.StartTime = desc.StartTime;
			newSample.Pan = note.Pan;
			newSample.Volume = note.Volume;
			newSample.Index = desc.KeysoundIndex;

			m_autoSamples.push_back(newSample);
			desc.KeysoundIndex = -1;
		}

		m_noteDescs.push_back(desc);
	}

	std::sort(m_noteDescs.begin(), m_noteDescs.end(), [](auto& a, auto& b) {
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

	UpdateGamePosition();
	UpdateNotes();

	m_timingLineManager = chart->m_customMeasures.size() > 0 ? new TimingLineManager(this, chart->m_customMeasures) : new TimingLineManager(this);
	m_scoreManager = new ScoreManager();

	m_startClock = std::chrono::system_clock::now();

	m_timingLineManager->Init();
	m_state = GameState::NotGame;
	return true;
}

void RhythmEngine::SetKeys(Keys* keys) {
	for (int i = 0; i < 7; i++) {
		KeyMapping[i].key = keys[i];
	}
}

bool RhythmEngine::Start() { // no, use update event instead
	m_currentAudioPosition -= 3000;
	m_state = GameState::Playing;

	m_startClock = std::chrono::system_clock::now();
	return true;
}

bool RhythmEngine::Stop() { 
	m_state = GameState::PosGame;
	return true;
}

bool RhythmEngine::Ready() {
	return m_state == GameState::NotGame;
}

void RhythmEngine::Update(double delta) {
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;

	// Since I'm coming from Roblox, and I had no idea how to Real-Time sync the audio
	// I decided to use this method again from Roblox project I did in past.
	double last = m_currentAudioPosition;
	m_currentAudioPosition += (delta * m_rate) * 1000;

	// check difference between last and current audio position
	// if it's too big, then it means the game is lagging

	if (m_currentAudioPosition - last > 1000 * 5) {
		assert(false); // TODO: Handle this
	}

	if (m_currentAudioPosition > m_audioLength + 2500) { // Avoid game ended too early
		m_state = GameState::PosGame;
		::printf("Audio stopped!\n");
	}

	if (static_cast<int>(m_currentAudioPosition) % 1000 == 0) {
		m_noteImageIndex = (m_noteImageIndex + 1) % m_noteMaxImageIndex;
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
		if (m_currentAudioPosition >= sample.StartTime) {
			if (sample.StartTime - m_currentAudioPosition < 5) {
				GameAudioSampleCache::Play(sample.Index, (int)round(sample.Volume * m_audioVolume), (int)round(sample.Pan * 100));
			}

			m_currentSampleIndex++;
		}
		else {
			break;
		}
	}

	if (m_is_autoplay) {
		auto frame = GetAutoplayAtThisFrame(m_currentAudioPosition);

		for (auto& frame : frame.KeyDowns) {
			m_tracks[frame.Lane]->OnKeyDown();
		}

		for (auto& frame : frame.KeyUps) {
			m_tracks[frame.Lane]->OnKeyUp();
		}
	}

	auto currentTime = std::chrono::system_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startClock);
	m_PlayTime = static_cast<int>(elapsedTime.count() / 1000);
}

void RhythmEngine::Render(double delta) {
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;
	
	m_timingLineManager->Render(delta);

	for (auto& it : m_tracks) {
		it->Render(delta);
	}
}

void RhythmEngine::Input(double delta) {
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;
}

void RhythmEngine::OnKeyDown(const KeyState& state) {
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;

	if (state.key == Keys::F3) {
		m_scrollSpeed -= 10;
	}
	else if (state.key == Keys::F4) {
		m_scrollSpeed += 10;
	}

	if (!m_is_autoplay) {
		for (auto& key : KeyMapping) {
			if (key.second.key == state.key) {
				key.second.isPressed = true;

				if (key.first < m_tracks.size()) {
					m_tracks[key.first]->OnKeyDown();
				}
			}
		}
	}
}

void RhythmEngine::OnKeyUp(const KeyState& state) {
	if (m_state == GameState::NotGame || m_state == GameState::PosGame) return;

	if (!m_is_autoplay) {
		for (auto& key : KeyMapping) {
			if (key.second.key == state.key) {
				key.second.isPressed = false;

				if (key.first < m_tracks.size()) {
					m_tracks[key.first]->OnKeyUp();
				}
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
	double scrollingFactor = 1920.0 / 1366.0;
	float virtualRatio = (float)(m_virtualResolution.Y / m_gameResolution.Y);
	float value = (float)((speed / 10.0) / (20.0 * m_rate) * scrollingFactor * virtualRatio);

	return value;
}

double RhythmEngine::GetBPMAt(double offset) const {
	auto& bpms = m_currentChart->m_bpms;
	int min = 0, max = (int)(bpms.size() - 1);

	if (max == 0) {
		return bpms[0].Value;
	}

	while (min <= max) {
		int mid = (min + max) / 2;

		bool afterMid = mid < 0 || bpms[mid].StartTime <= offset;
		bool beforeMid = mid + 1 >= bpms.size() || bpms[mid + 1].StartTime > offset;

		if (afterMid && beforeMid) {
			return bpms[mid].Value;
		}
		else if (afterMid) {
			max = mid - 1;
		}
		else {
			min = mid + 1;
		}
	}

	return bpms[0].Value;
}

double RhythmEngine::GetCurrentBPM() const {
	return m_currentBPM;
}

double RhythmEngine::GetSongRate() const {
	return m_rate;
}

double RhythmEngine::GetAudioLength() const {
	return m_audioLength;
}

int RhythmEngine::GetGameVolume() const {
	return m_audioVolume;
}

GameState RhythmEngine::GetState() const {
	return m_state;
}

ScoreManager* RhythmEngine::GetScoreManager() const {
	return m_scoreManager;
}

std::vector<TimingInfo> RhythmEngine::GetBPMs() const {
	return m_currentChart->m_bpms;
}

std::vector<TimingInfo> RhythmEngine::GetSVs() const {
	return m_currentChart->m_svs;
}

double RhythmEngine::GetElapsedTime() const { // Get game frame
	return static_cast<double>(SDL_GetTicks()) / 1000.0;
}

int RhythmEngine::GetPlayTime() const { // Get game time
	return m_PlayTime;
}

int RhythmEngine::GetNoteImageIndex() {
	return m_noteImageIndex;
}

int RhythmEngine::GetGuideLineIndex() const {
	return m_guideLineIndex;
}

void RhythmEngine::SetGuideLineIndex(int idx) {
	m_guideLineIndex = idx;
}

void RhythmEngine::UpdateNotes() {
	for (int i = m_currentNoteIndex; i < m_noteDescs.size(); i++) {
		auto& desc = m_noteDescs[i];
		
		if (m_currentAudioGamePosition + (3000.0 / GetNotespeed()) > desc.StartTime
			|| (m_currentTrackPosition - desc.InitialTrackPosition > GetPrebufferTiming())) {

			m_tracks[desc.Lane]->AddNote(&desc);

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

	m_currentTrackPosition = m_timings->GetOffsetAt(m_currentVisualPosition, m_currentSVIndex);// GetPositionFromOffset(m_currentVisualPosition, m_currentSVIndex);

	if (m_currentSVIndex > 0) {
		float svMultiplier = m_currentChart->m_svs[m_currentSVIndex - 1].Value;
		if (svMultiplier != m_currentSVMultiplier) {
			m_currentSVMultiplier = svMultiplier;
		}
	}

	if (m_currentBPMIndex > 0) {
		m_currentBPM = m_currentChart->m_bpms[m_currentBPMIndex - 1].Value;
	}
}

void RhythmEngine::UpdateVirtualResolution() {
	double width = GameWindow::GetInstance()->GetBufferHeight();
	double height = GameWindow::GetInstance()->GetBufferHeight();

	m_gameResolution = { width, height };

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
		double pos = ::round(svs[0].StartTime * m_currentChart->InitialSvMultiplier * 100);
		m_timingPositionMarkers.push_back(pos);

		for (int i = 1; i < svs.size(); i++) {
			pos += ::round((svs[i].StartTime - svs[i - 1].StartTime) * (svs[i - 1].Value * 100));

			m_timingPositionMarkers.push_back(pos);
		}
	}
}

ReplayFrameData RhythmEngine::GetAutoplayAtThisFrame(double offset) {
    ReplayFrameData data;

	for (int i = m_autoMinIndex; i < m_autoFrames.size(); i++) {
		auto& hit = m_autoFrames[i];

		if (offset >= hit.Time) {
			if (hit.Type == Autoplay::ReplayHitType::KEY_UP) {
				data.KeyUps.push_back(hit);
			} else {
				data.KeyDowns.push_back(hit);
			}

			m_autoMinIndex++;
		} else {
			break;
		}
	}

	return std::move(data);
}

int* RhythmEngine::GetLaneSizes() const {
	return (int*)&m_laneSize;
}

int* RhythmEngine::GetLanePos() const {
	return (int*)&m_lanePos;
}

void RhythmEngine::SetHitPosition(int offset) {
	m_hitPosition = offset;
}

void RhythmEngine::SetLaneOffset(int offset) {
	m_laneOffset = offset;
}

int RhythmEngine::GetHitPosition() const {
	return m_hitPosition;
}

Vector2 RhythmEngine::GetResolution() const {
	return m_gameResolution;
}

Rect RhythmEngine::GetPlayRectangle() const {
	return m_playRectangle;
}

std::u8string RhythmEngine::GetTitle() const {
	return m_title;
}

TimingBase* RhythmEngine::GetTiming() const {
	return m_timings;
}

JudgeBase* RhythmEngine::GetJudge() const {
	return m_judge;
}

void RhythmEngine::Release() {
	for (int i = 0; i < m_tracks.size(); i++) {
		delete m_tracks[i];
	}

	delete m_timingLineManager;
	delete m_timings;
	delete m_scoreManager;
	delete m_judge;

	NoteImageCacheManager::Release();
	GameNoteResource::Dispose();
	GameAudioSampleCache::StopAll();
}
