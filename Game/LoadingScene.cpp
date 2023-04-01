#include "LoadingScene.h"
#include <iostream>
#include <filesystem>

#include "EnvironmentSetup.hpp"
#include "GameScenes.h"
#include "Data/osu.hpp"

LoadingScene::LoadingScene() {
	m_background = nullptr;
	m_counter = 0;
}

LoadingScene::~LoadingScene() {
	
}

void LoadingScene::Update(double delta) {
	m_counter += delta;

#ifdef _DEBUG
	m_counter = 5;
#endif

	if (m_counter > 2.5) {
		SceneManager::ChangeScene(GameScene::GAME);
	}
}

void LoadingScene::Render(double delta) {
	if (m_background) {
		m_background->Draw();
	}
}

bool LoadingScene::Attach() {
#ifndef _DEBUG
	try {
		std::string file = EnvironmentSetup::Get("FILE");
		Osu::Beatmap beatmap(file);

		if (!beatmap.IsValid()) {
			MessageBoxA(NULL, "Failed to load beatmap!", "EstEngine Error", MB_ICONERROR);
			return false;
		}

		if (beatmap.Mode != 3) {
			MessageBoxA(NULL, "This beatmap is not a mania beatmap!", "EstEngine Error", MB_ICONERROR);
			return false;
		}

		Osu::OsuEvent* bg = nullptr;
		for (auto it = beatmap.Events.begin(); it != beatmap.Events.end();) {
			auto& event = *it;

			if (event.Type == Osu::OsuEventType::Background) {
				bg = &(*it);
				break;
			}
			else {
				it++;
			}
		}

		if (bg) {
			std::filesystem::path imgPath = beatmap.FileDirectory;
			imgPath /= bg->params[0];

			if (std::filesystem::exists(imgPath)) {
				m_background = new Texture2D(imgPath.string());
				m_background->Size = UDim2::fromOffset(800, 600);
			}
		}
	}
	catch (std::exception& e) {

	}
#endif

	return true;
}

bool LoadingScene::Detach() {
	if (m_background) {
		delete m_background;
		m_background = nullptr;
	}

	return true;
}
