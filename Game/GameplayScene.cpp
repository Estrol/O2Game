#include "GameplayScene.h"
#include "./Engine/NoteImageCacheManager.hpp"
#include <iostream>
#include <unordered_map>
#include "EnvironmentSetup.hpp"
#include <future>

GameplayScene::GameplayScene() : Scene::Scene() {
	m_keyLighting = {};
	m_keyButtons = {};
	m_keyState = {};
	m_game = nullptr;
}

void GameplayScene::Update(double delta) {
	static bool started = false;
	
	if (!started && m_game->Ready()) {
		started = true;

		std::async(std::launch::async | std::launch::deferred, &RhythmEngine::Start, m_game);
		std::cout << "async func" << std::endl;
	}

	m_game->Update(delta);
}

void GameplayScene::Render(double delta) {
	m_playfieldBG->Draw();
	
	for (auto& it : m_keyState) {
		if (it.second) {
			m_keyLighting[it.first]->Draw();
			m_keyButtons[it.first]->Draw();
		}
	}

	m_game->Render(delta);
}

void GameplayScene::Input(double delta) {
	
}

void GameplayScene::OnKeyDown(const KeyState& state) {
	m_game->OnKeyDown(state);
}

void GameplayScene::OnKeyUp(const KeyState& state) {
	m_game->OnKeyUp(state);
}

bool GameplayScene::Attach() {
	for (int i = 0; i < 7; i++) {
		m_keyState[i] = false;
	}

	{
		auto playfieldFile = GamePlayingResource::GetFile("Note_BG.ojs");
		if (!playfieldFile) {
			MessageBoxA(NULL, "Failed to load Note_BG.ojs", "Error", MB_OK);
			return false;
		}

		auto handle = GamePlayingResource::LoadFileData(playfieldFile);
		if (!handle) {
			MessageBoxA(NULL, "Failed to load Note_BG.ojs as OJS!", "Error", MB_OK);
			return false;
		}

		OJS* ojs = (OJS*)handle;
		m_playfieldBG = new O2Texture(ojs->Frames[0].get());

		delete handle;
	}

	{
		auto keyLightingFile = GamePlayingResource::GetFile("Keyeffect.ojs");
		auto keyButtonFile = GamePlayingResource::GetFile("KeydownImage.ojs");

		if (!keyLightingFile || !keyButtonFile) {
			MessageBoxA(NULL, "Failed to load Keyeffect.ojs or KeydownImage.ojs", "Error", MB_OK);
			return false;
		}

		auto handle1 = GamePlayingResource::LoadFileData(keyLightingFile);
		auto handle2 = GamePlayingResource::LoadFileData(keyButtonFile);

		if (!handle1 || !handle2) {
			MessageBoxA(NULL, "Failed to load Keyeffect.ojs or KeydownImage.ojs as OJS!", "Error", MB_OK);
			return false;
		}

		OJS* keyLighting = (OJS*)handle1;
		OJS* keyButton = (OJS*)handle2;

		for (int i = 0; i < 7; i++) {
			O2Texture* lighting = new O2Texture(keyLighting->Frames[i].get());
			O2Texture* button = new O2Texture(keyButton->Frames[i].get());

			m_keyLighting.insert({ i, lighting });
			m_keyButtons.insert({ i, button });
		}
	}

	{
		std::string file = EnvironmentSetup::Get("FILE");
		const char* bmsfile[] = {".bms", ".bme", ".bml"};

		Chart* chart = nullptr;
		if (file.find(bmsfile[0]) != std::string::npos || file.find(bmsfile[1]) != std::string::npos || file.find(bmsfile[2]) != std::string::npos) {
			BMS::BMSFile beatmap;
			beatmap.Load(file);
			
			if (!beatmap.IsValid()) {
				MessageBoxA(NULL, "Failed to load BMS!", "EstEngine Error", MB_ICONERROR);
				return false;
			}

			chart = new Chart(beatmap);
			m_game = new RhythmEngine();
		}
		else {
			Osu::Beatmap beatmap(file);

			if (!beatmap.IsValid()) {
				MessageBoxA(NULL, "Failed to load beatmap!", "EstEngine Error", MB_ICONERROR);
				return false;
			}

			std::string title = "EstRhythm - " + beatmap.Artist + " - " + beatmap.Title;
			Window::GetInstance()->SetWindowTitle(title);

			Chart* chart = new Chart(beatmap);
			m_game = new RhythmEngine();
		}

		if (!m_game) {
			MessageBoxA(NULL, "Failed to load game!", "EstEngine Error", MB_ICONERROR);
			return false;
		}

		m_game->ListenKeyEvent([&](int lane, bool state) {
			m_keyState[lane] = state;
		});

		m_game->Load(chart);
	}

	return true;
}

bool GameplayScene::Detach() {
	for (auto& it : m_keyButtons) {
		delete it.second;
	}

	for (auto& it : m_keyLighting) {
		delete it.second;
	}

	delete m_game;

	NoteImageCacheManager::Release();
	return true;
}
