#include "GameplayScene.h"
#include "./Engine/NoteImageCacheManager.hpp"
#include <iostream>
#include <unordered_map>
#include "EnvironmentSetup.hpp"

GameplayScene::GameplayScene() : Scene::Scene() {
	m_keyLighting = {};
	m_keyButtons = {};
	m_game = nullptr;

	m_keyMap = {
		{ManiaKeys::MANIA_KEY_1, {Keys::A, false}},
		{ManiaKeys::MANIA_KEY_2, {Keys::S, false}},
		{ManiaKeys::MANIA_KEY_3, {Keys::D, false}},
		{ManiaKeys::MANIA_KEY_4, {Keys::Space, false}},
		{ManiaKeys::MANIA_KEY_5, {Keys::J, false}},
		{ManiaKeys::MANIA_KEY_6, {Keys::K, false}},
		{ManiaKeys::MANIA_KEY_7, {Keys::L, false}},
	};
}

void GameplayScene::Update(double delta) {
	static double waitTime = 0;
	static bool started = false;
	
	if ((waitTime += delta) > 5 && !started) {
		started = true;

		m_game->Start();
	}

	m_game->Update(delta);
}

void GameplayScene::Render(double delta) {
	m_playfieldBG->Draw();
	
	for (auto& it : m_keyMap) {
		if (it.second.isPressed) {
			m_keyLighting[it.first]->Draw();
			m_keyButtons[it.first]->Draw();
		}
	}

	m_game->Render(delta);
}

void GameplayScene::Input(double delta) {
	
}

void GameplayScene::OnKeyDown(const KeyState& state) {
	for (auto& it : m_keyMap) {
		if (it.second.key == state.key) {
			it.second.isPressed = true;
		}
	}

	m_game->OnKeyDown(state);
}

void GameplayScene::OnKeyUp(const KeyState& state) {
	for (auto& it : m_keyMap) {
		if (it.second.key == state.key) {
			it.second.isPressed = false;
		}
	}

	m_game->OnKeyUp(state);
}

bool GameplayScene::Attach() {
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

			m_keyLighting.insert({ (ManiaKeys)i, lighting });
			m_keyButtons.insert({ (ManiaKeys)i, button });
		}
	}

	{
		std::string file = EnvironmentSetup::Get("FILE");
		Osu::Beatmap beatmap(file);

		if (!beatmap.IsValid()) {
			MessageBoxA(NULL, "Failed to load beatmap!", "EstEngine Error", MB_ICONERROR);
			return false;
		}

		std::string title = "Estrol RhythmPlayer: " + beatmap.Artist + " - " + beatmap.Title;
		Window::GetInstance()->SetWindowTitle(title);

		Chart* chart = new Chart(beatmap);
		m_game = new RhythmEngine();
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
